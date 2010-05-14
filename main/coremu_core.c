/* @(#)coremu_core.c
 *
 * core emulation support for COREMU parallel
 * emulation library.
 *
 * contact:
 * Xi Wu (wuxi@fudan.edu.cn)
 * Zhaoguo Wang (tigerwang1986@gmail.com)  */

#define DEBUG_COREMU     0
#define VERBOSE_COREMU   1

#define _GNU_SOURCE             /* for some GNU specific interfaces */

#include "coremu_utils.h"
#include "coremu_hw.h"
#include "coremu_timer.h"
#include "coremu_sched.h"

#if IOREQ_LOCK_FREE
#include <coremu_hw_lockfree.h>
#endif
/* pause condition */
pthread_cond_t pause_cond=COREMU_COND_INITIALIZER;
pthread_mutex_t pause_mutex=COREMU_LOCK_INITIALIZER;

/* list of cores */
cores_head_t coremu_cores;

/* coremu cores init related */
static volatile int init_done = false;

/* coremu core itself for each thread */
__thread CMCore* core_self = NULL;
__thread pthread_mutexattr_t attr;
static pthread_attr_t thr_attr;

/* coremu profiling */
int cm_profiling_p = 0;

/* coremu retry profiling */
__thread unsigned long int cm_retry_num = 0;

/* the per core signal blocking set */
//__thread sigset_t cm_blk_sigset;

extern int cm_adaptive_intr_delay;
extern int cm_intr_delay_step;
void coremu_init(int smp_cpus, msg_handler msg_fn)
{
    cm_print("\nTIMERRTSIG:\t\t%d"
             "\nCOREMU_TIMER_SIGNAL:\t%d"
             "\nCOREMU_TIMER_ALARM:\t%d"
             "\nCOREMU_SIGNAL:\t\t%d"
             "\nCOREMU_AIO_SIG:\t\t%d\n",
             TIMERRTSIG, COREMU_TIMER_SIGNAL,
             COREMU_TIMER_ALARM, COREMU_SIGNAL, COREMU_AIO_SIG);

    /* step 0: init scheduling support */
    coremu_init_sched_all();
    cm_profiling_p = COREMU_PROFILE;

    /* the adaptive intr delay mechanism works well 
        when core's number is more than 64 (test enviroment R900)*/
    if(smp_cpus > 64)
        cm_adaptive_intr_delay = 1;
    else
        cm_adaptive_intr_delay = 0;
    
     /**
      * we define the step is 1, if the number of cores
      * is not more than 128
      */	
     if(cm_adaptive_intr_delay)
	    cm_intr_delay_step = (smp_cpus + 127)/128;
    
    /* step 1: init the global core list */
    TAILQ_INIT(&coremu_cores);

    /* step 2: init the coremu timer thread */
    
    /* clear the block signal set */
    //sigemptyset(&cm_blk_sigset);
    
    /* block TIMERRTSIG */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, TIMERRTSIG);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* add the block signal to block signal set */
    //sigaddset(&cm_blk_sigset, TIMERRTSIG);
    
    coremu_start_timer_thread();
    coremu_init_hw(smp_cpus);

    /* step 3: register CORMEU signal handling */
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_flags |= SA_SIGINFO;
    act.sa_sigaction = msg_fn;

    sigaction(COREMU_SIGNAL, &act, NULL);
}

CMCore *coremu_core_init(void* opaque)
{
    int err = 0;

    /* step 1: allocate the core */
    CMCore *core = (cm_core_t *) qemu_mallocz(sizeof(cm_core_t));

    /* step 2: init the hardware event queue and its lock */
    err = pthread_mutexattr_init(&attr);
    cm_assert((err == 0), "cannot init the attribute");

    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    cm_assert((err == 0), "cannot set mutex type");

#if INTR_LOCK_FREE
    core->intr_queue = new_queue();
#elif INTR_LOCK
    TAILQ_INIT(&core->intr_queue);
    pthread_mutex_init(&core->intr_lock, &attr);
    core->intr_count = 0;
#else
# error "No HWE queue?"
#endif

    /* step 3: init opaque state */
    core->opaque = opaque;

    /* step 4: set core state to run */
    core->state = STATE_RUN;

    /* reset the profile */
    core->core_profile.retry_num_addr = &cm_retry_num;
    
    /* step 5: put this core into cores tailq */
    TAILQ_INSERT_TAIL(&coremu_cores, core, cores);

    return core;
}

void coremu_run_all_cores(thr_start_routine thr_fn, void *arg)
{
    CMCore *cur;
    int err;

    pthread_attr_init(&thr_attr);
    pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&thr_attr, SCHED_RR);

    TAILQ_FOREACH(cur, &coremu_cores, cores)
    {
        err = pthread_create(&cur->coreid, &thr_attr, thr_fn, arg);
        cm_assert((! err), "pthread creation fails...");
    }

    pthread_attr_destroy(&thr_attr);
    init_done = true;
}

int coremu_init_done_p()
{
    int done_flag;
    done_flag = init_done;

    return done_flag;
}

CMCore *coremu_get_first_core()
{
    cm_assert((! TAILQ_EMPTY(&coremu_cores)), "NO cores?!");
    return TAILQ_FIRST(&coremu_cores);
}

CMCore *coremu_get_self()
{
    cm_assert((! TAILQ_EMPTY(&coremu_cores)), "NO cores?!");
    coremu_assert_core_thr();

    if(! core_self) {
        core_self = coremu_get_core(pthread_self());
    }

    return core_self;
}

void coremu_assert_core_thr()
{
    if(pthread_self() == coremu_get_hw_id()) {
        coremu_backtrace();
        assert(0);
    }
    return;
}

CMCore *coremu_get_core(core_t coreid)
{
    cm_assert((! TAILQ_EMPTY(&coremu_cores)),
              "NO cores?!");

    CMCore *cur;
    TAILQ_FOREACH(cur, &coremu_cores, cores) {
        if(cur->coreid == coreid) {
            return cur;
        }
    }

    cm_print("tid[%lu] fatal error", (unsigned long int) coremu_gettid());
    assert(0);
}

cores_head_t *coremu_get_core_tailq(void)
{
    return &coremu_cores;
}

void coremu_wait_init(void)
{
    struct timespec tsp;
    while(! init_done) {
        maketimeout(&tsp, 1);   /* 1 second per check */
        nanosleep(&tsp, NULL);
    }
}

int coremu_wait_tid(CMCore *core, void **val_ptr)
{
    return pthread_join(core->coreid, val_ptr);
}

void coremu_dump_prio()
{
    CMCore *cur = NULL;
    cores_head_t *core_head = coremu_get_core_tailq();

    int policy;
    struct sched_param param;
    TAILQ_FOREACH(cur, core_head, cores)
    {
        assert(! pthread_getschedparam(cur->coreid,
                                       &policy, &param));
        assert(policy == CM_SCHED_POLICY);

        cm_print("core[%lu]: policy[%d], prio[%d]",
                 cur->coreid, policy, param.sched_priority);
    }
}

void coremu_dump_core_profiling_info(void)
{
    CMCore *cur = NULL;
    cores_head_t *core_head = coremu_get_core_tailq();

    TAILQ_FOREACH(cur, core_head, cores)
    {
        fprintf(stderr, "(core[%lu])", cur->coreid);
        coremu_dump_profile(&cur->core_profile, "");
    }
}

void coremu_dump_profile(cm_profile_t *profile, const char *msg)
{
    fprintf(stderr,
            "%s\t%lu/%lu\t%lu/%lu\t%lu/%lu\t%lu/%lu\t%lu/%lu\t%lu/%lu\t%lu/%lu\t%lu/%lu\n",
            msg,
            profile->intr_send[I8259_EVENT], profile->intr_recv[I8259_EVENT],
            profile->intr_send[IOAPIC_EVENT], profile->intr_recv[IOAPIC_EVENT],
            profile->intr_send[IPI_EVENT], profile->intr_recv[IPI_EVENT],
            profile->intr_send[DIRECT_INTR_EVENT], profile->intr_recv[DIRECT_INTR_EVENT],
            profile->intr_send[CPU_EXIT_EVENT], profile->intr_recv[CPU_EXIT_EVENT],
            profile->intr_send[TB_INVAL_EVENT], profile->intr_recv[TB_INVAL_EVENT],
            profile->intr_send[TB_INVAL_FAST_EVENT], profile->intr_recv[TB_INVAL_FAST_EVENT],
            profile->intr_send[CPU_SHUTDOWN_EVENT], profile->intr_recv[CPU_SHUTDOWN_EVENT]);
    fflush(stderr);
}

void coremu_dump_timer_debug_info(cm_timer_debug_t *debug_info, const char *msg)
{
    fprintf(stderr, 
            "%sLAPIC TIMER:\nCREATED: %d\nCNT:       %ld\nREARM CNT: %ld\n", 
             msg, debug_info->created_timer, debug_info->lapic_timer_cnt, debug_info->rearm_cnt);   

     fprintf(stderr, 
            "Last Flag: 0x%x\nLast Current Time: 0x%lx\nLast Expire  Time: 0x%lx\n", 
             debug_info->last_flags, debug_info->last_current_time, debug_info->last_expire_time); 

    
    fflush(stderr);
}


void coremu_dump_debug_info(void)
{
    CMCore *cur = NULL;
    cores_head_t *core_head = coremu_get_core_tailq();

    TAILQ_FOREACH(cur, core_head, cores)
    {
       fprintf(stderr,"CPU[%d] Debug Info\n", cur->serial);
       coremu_dump_timer_debug_info(&cur->debug_info, " ");
    }
}
void coremu_core_exit(void *value_ptr)
{
    pthread_exit(value_ptr);
}

void coremu_pause_core()
{
	CMCore *self = coremu_get_self();
	coremu_mutex_lock(&pause_mutex,"coremu_pause_core");
	if(self->state==STATE_RUN){
		self->state=STATE_PAUSE;
		coremu_cond_wait(&pause_cond, &pause_mutex);
		self->state=STATE_RUN;
	}
	coremu_mutex_unlock(&pause_mutex,"coremu_pause_core");
}

void coremu_wait_pause(CMCore *core)
{
	coremu_mutex_lock(&pause_mutex,"coremu_wait_pause");
	while(core->state!=STATE_PAUSE){
		coremu_mutex_unlock(&pause_mutex,"coremu_wait_pause");
		coremu_mutex_lock(&pause_mutex,"coremu_wait_pause");
	}
	coremu_mutex_unlock(&pause_mutex,"coremu_wait_pause");
}

void coremu_restart_all_cores()
{
	coremu_cond_broadcast(&pause_cond);
}
