/* @(#)coremu_intr.c
 *
 * Interfaces for manipulating interrupts in COREMU.
 * (e.g. interrupt from device to core, and
 * IPI between cores).
 *
 */

#define DEBUG_COREMU    0
#define VERBOSE_COREMU  1

#include "coremu_utils.h"
#define NEED_CPU_H
#include "target-qemu.h"
#include "coremu_hw.h"
#include "coremu_intr.h"
#include "coremu_sched.h"

#define INTR_NUM_THRESHOLD   50    /* notify a CORE if many pending signals */
#define INTR_TIME_THRESHOLD  1000   /* 1ms */
/* hardware profile */
cm_profile_t hw_profile;

/* the flag of adpative signal delay */
int cm_adaptive_intr_delay;

/* the step of intr delay, the signal accept time must be
 * more than cm_intr_delay_step * 10us
 */
int cm_intr_delay_step;


/* Signal-unsafe. block the signal in the lock free function */
void coremu_put_intr(void *e, size_t size, cm_core_t *core)
{
    cm_intr_t *cm_intr;
    cm_intr = qemu_mallocz(sizeof(cm_intr_t));

    void *opaque = qemu_mallocz(size);
    memcpy(opaque, e, size);

    cm_intr->opaque = opaque;

#if INTR_LOCK_FREE

    enqueue(core->intr_queue, (uint64_t) cm_intr);

#elif INTR_LOCK

    coremu_mutex_lock(&core->intr_lock,
                      "cannot acquire hw event queue lock");
    TAILQ_INSERT_TAIL(&core->intr_queue, cm_intr, intr);
    core->intr_count++;
    coremu_mutex_unlock(&core->intr_lock,
                        "cannot release hw event queue lock");

#else
# error "No interrupt queue?"
#endif
}

/* Signal-unsafe. block the signal in the lock free function */
void *coremu_get_intr(cm_core_t *core)
{
    cm_intr_t *cm_intr;
    void *opaque;
    
    if(! coremu_intr_p(core))
        return NULL;

#if INTR_LOCK_FREE

    int ret; uint64_t tmp;
    ret = dequeue(core->intr_queue, &tmp);
    assert(ret);
    cm_intr = (cm_intr_t *)tmp;

#elif INTR_LOCK

    coremu_mutex_lock(&core->intr_lock,
                      "cannot acquire hw event queue lock");
    cm_intr = TAILQ_FIRST(&core->intr_queue);
    TAILQ_REMOVE(&core->intr_queue, cm_intr, intr);
    core->intr_count--;
    coremu_mutex_unlock(&core->intr_lock,
                        "cannot release hw event queue lock");

#else
# error "No interrupt queue?"
#endif

    opaque = cm_intr->opaque;
    qemu_free(cm_intr);
    
    return opaque;
}

uint64_t coremu_intr_get_size(cm_core_t *core)
{
#if INTR_LOCK_FREE
    return ms_queue_get_size(core->intr_queue);
#elif INTR_LOCK
    return core->intr_count;
#endif
}

int coremu_intr_p(cm_core_t *core)
{
    return coremu_intr_get_size(core);
}

void coremu_prepare_intr(qemu_intr_t *intr,
                         int source, int level,
                         int mask, int vector_num,
                         int triger_mode, int deliver_mode, int target_id)
{
    intr->source = source;
    intr->level  = level;
    intr->mask   = mask;
    intr->vector_num = vector_num;
    intr->triger_mode = triger_mode;
    intr->deliver_mode = deliver_mode;
    intr->target_id = target_id;
    return;
}

void coremu_prepare_tbinval_intr(qemu_intr_t *intr,
                                 int source,
                                 target_phys_addr_t start,
                                 target_phys_addr_t end,
                                 int len, int is_cpu_write_access,
                                 uint32_t *tb_invalidate_ack)
{
    intr->source = source;
    intr->addr_range.start = start;
    intr->addr_range.end = end;
    intr->addr_range.len = len;
    intr->addr_range.is_cpu_write_access = is_cpu_write_access;
    intr->tb_invalidate_ack_addr = tb_invalidate_ack;
}

/**
 * Send an interrupt and notify the accept core
 * Here we use apdaptive signal delay mechanism
 * But this mechanism seems to be wonderful when number of emulated
 * cores is more than 128 (test enviroment R900)
 */
void coremu_notify_intr(void *e, size_t size, cm_core_t *core)
{
    uint64_t pending_intr;
    
    if(cm_profiling_p) {
        qemu_intr_t *intr = (qemu_intr_t *)e;
        assert(intr->source < CM_INTR_CNT);
        if(coremu_hw_thr_p()) {
            atomic_inc64(&hw_profile.intr_send[intr->source]);
        } else {
            cm_core_t *self = coremu_get_self();
            assert(self != NULL);
            atomic_inc64(&self->core_profile.intr_send[intr->source]);
        }
    }

    coremu_put_intr(e, size, core);

    pending_intr = coremu_intr_get_size(core);

/*  here need to do somethings that make the thresh hold to be  more smart!!!*/
//	if(pending_intr > 10)
//		core->intr_thresh_hold = 0;


  if(core->sig_pending)
  {
	if(core->intr_thresh_hold < 100)
		core->intr_thresh_hold += 10;
	return;
  }
   

  if(!cm_adaptive_intr_delay || core->state==STATE_HALT
		  || pending_intr > core->intr_thresh_hold
		  || (core->state == STATE_PAUSE && pending_intr > 10))
  {
	  core->sig_pending = 1;
	  core->state = STATE_RUN;

	  coremu_thread_setpriority(PRIO_PROCESS, core->tid, high_prio);
	  pthread_kill(core->coreid, COREMU_SIGNAL);
  }

  
    
}

/* broadcast event to core cores */
void coremu_broadcast_intr(void *e, size_t size)
{
    cores_head_t *head = coremu_get_core_tailq();
    cm_assert(! TAILQ_EMPTY(head), "empty hw event queue?!");

    /* XXX: cannot free the event if we append the same
       event to event queue of each thread.

       Currently solve this problem by duplicating event
       to each queue. Better idea for this ??? */

    cm_core_t *cur;

    TAILQ_FOREACH(cur, head, cores) {
        coremu_notify_intr(e, size, cur);
    }
}

/* broadcast event to other cores */
void coremu_broadcast_intr_other(void *e, size_t size)
{
    cores_head_t *head = coremu_get_core_tailq();
    cm_assert(! TAILQ_EMPTY(head), "empty hw event queue?!");

    /* XXX: cannot free the event if we append the same
       event to event queue of each thread.

       Currently solve this problem by duplicating event
       to each queue. Better idea for this ??? */

    cm_core_t *self, *cur;
    self = coremu_get_self();
    TAILQ_FOREACH(cur, head, cores) {
        if(cur != self)
            coremu_notify_intr(e, size, cur);
    }
}


