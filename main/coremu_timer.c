/*
 * COREMU Parallel Emulator Framework
 *
 * Timer abstraction in COREMU system.
 *
 * Copyright (C) 2010 PPI, Fudan Univ. <http://ppi.fudan.edu.cn/system_research_group>
 *
 * Authors:
 *  Zhaoguo Wang    <zgwang@fudan.edu.cn>
 *  Yufei Chen      <chenyufei@fudan.edu.cn>
 *  Ran Liu         <naruilone@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#define DEBUG_COREMU              0
#define VERBOSE_COREMU            1
#define BIND_TIMER_THR            0      /* bind timer thread to a specific CORE? */
#define TIMER_PERIODIC_SLEEP      1      /* sleep sometime instead of busy loop? */
#define LAPIC_TIME_DEBUG          0

#include "coremu_utils.h"
#include "coremu_timer.h"

#define sigev_notify_thread_id _sigev_un._tid
#define TIMER_WAIT_TIME    (10)
#define handle_error_en(en, msg)   \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#if TIMER_THREAD_ENABLE

static void local_alarm_handler(int signo,
                           siginfo_t *info, void *context);
static void *timer_fn(void* arg);
static volatile int timer_init_done = 0;

#endif

int coremu_local_timer_create(int signo,
                              void *opaque,
                              timer_t *host_timer_ptr,
                              cm_local_alarm_t **lalarm_ptr,
                              pthread_t thrid)
{
    /* init the thread local alarm */
    cm_local_alarm_t *lalarm = NULL;
    lalarm = qemu_mallocz(sizeof(cm_local_alarm_t));
    lalarm->opaque = opaque;
    lalarm->signo = signo;
    lalarm->tid = thrid;

    *lalarm_ptr = lalarm;

    /* create alarm timer using 'timer_create' */
    struct sigevent ev;
    ev.sigev_value.sival_ptr = (void *) (lalarm);

#if TIMER_THREAD_ENABLE
	ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = TIMERRTSIG;
#else
	ev.sigev_notify = SIGEV_THREAD_ID;
	ev.sigev_signo = signo;
	ev.sigev_notify_thread_id = coremu_gettid();
#endif

    return timer_create(CLOCK_REALTIME, &ev, host_timer_ptr);
}

void coremu_start_timer_thread()
{
#if TIMER_THREAD_ENABLE
    /* XXX: other threads block TIMERRTSIG */
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, TIMERRTSIG);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* first create a specific timer thread */
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_fn, NULL);

    /* wait the timer thread to be OK */
    struct timespec tsp;
    while(! timer_init_done) {
        maketimeout(&tsp, 1);   /* 1 second per check */
        nanosleep(&tsp, NULL);
    }
#endif
}

#if TIMER_THREAD_ENABLE

/* signal handler for local alarm */
static void local_alarm_handler(int signo,
                                siginfo_t *info, void *context)
{
    assert((signo == TIMERRTSIG));

    /* handle the timer event */
    cm_local_alarm_t *ltimer = (cm_local_alarm_t*)info->si_value.sival_ptr;

    /* send the coremu signal to the core */
    pthread_kill(ltimer->tid, ltimer->signo);
}

static void *timer_fn(void* arg)
{
    sigset_t set;

#if DEBUG_COREMU
    cm_print("Timer Thread tid[%lu]",
             (unsigned long int)coremu_gettid());
#endif

#if BIND_TIMER_THR
    /* bind the timer thread to specific core,
       and give it high priority */
    int host_cpus = coremu_get_hostcpu();
    cm_print(">>> [Host] %d processors available <<<",
             host_cpus);

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET((host_cpus - 1), &cpuset);

    int s = pthread_setaffinity_np(pthread_self(),
                                   sizeof(cpu_set_t), &cpuset);
    if(s != 0)
        handle_error_en(s, "pthread_setaffinity_np");
#endif

    /* set the priority of timer thread.
       NOTE: since timing is critical, we set it
       to highest priority */
    //setpriority(PRIO_PROCESS, 0, coremu_get_maxprio());

    /* block all but TIMERRTSIG signal */
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    sigemptyset(&set);
    sigaddset(&set, TIMERRTSIG);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);


    /* register the signal handler */
    struct sigaction act;
    sigfillset(&act.sa_mask);   /* mask all signals on entry */
    act.sa_flags = 0;
    act.sa_flags |= SA_SIGINFO;
    act.sa_sigaction = local_alarm_handler;

    sigaction(TIMERRTSIG, &act, NULL);

    /* init is ready */
    timer_init_done = true;

#if DEBUG_COREMU
    cm_print("Timer is ok");
#endif

    /* nanosleep to wait for signal */
    struct timespec tsp;
    while(true) {
#if TIMER_PERIODIC_SLEEP
        maketimeout(&tsp, TIMER_WAIT_TIME);
        nanosleep(&tsp, NULL);
#endif
    }
}

#endif
