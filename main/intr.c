/*
 * COREMU Parallel Emulator Framework
 *
 * Interfaces for manipulating interrupts in COREMU. (e.g. interrupt from
 * device to core, and IPI between cores).
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

/*#define DEBUG_COREMU*/
#define VERBOSE_COREMU

#include <signal.h>
#include <stdbool.h>
#include "utils.h"
#include "coremu-hw.h"
#include "coremu-intr.h"
#include "coremu-sched.h"
#include "core.h"

#define MAX_INTR_THRESHOLD       10 

#define MIN_SIG_INTERVAL    5000    /* the minimal signal handler execution interval */
#define MAX_SIG_INTERVAL    500000

#define MAX_THRS_PER_CORE \
    (coremu_get_targetcpu() + coremu_get_hostcpu() - 1) / coremu_get_hostcpu()

static inline uint64_t coremu_intr_get_size(CMCore *core)
{
    return ms_queue_get_size(core->intr_queue);
}

static inline int coremu_intr_p(CMCore *core)
{
    return coremu_intr_get_size(core) != 0;
}

/* Insert an intrrupt into queue.
 * Signal-unsafe. block the signal in the lock free function */
static inline void coremu_put_intr(CMCore *core, void *e)
{
    enqueue(core->intr_queue, (long) e);
}

/* Get the first interrupt from queue.
 * Signal-unsafe. block the signal in the lock free function */
static void *coremu_get_intr(CMCore *core)
{
    if (!coremu_intr_p(core))
        return NULL;

    unsigned long intr;
    /* XXX the queue implementation may have bug.
     * It shouldn't be empty when there're pending interrupts. */
    assert(dequeue(core->intr_queue, &intr));

    return (void *)intr;
}

static void coremu_send_signal(CMCore *core)
{
    uint64_t pending_intr = coremu_intr_get_size(core);

    if(pending_intr > core->intr_thresh_hold) {
        if (!core->sig_pending) {
                core->sig_pending = 1;
                core->state = CM_STATE_RUN;
                coremu_thread_setpriority(PRIO_PROCESS, core->tid, high_prio);
                pthread_kill(core->thread, COREMU_SIGNAL);

        } else {
            if(core->intr_thresh_hold < MAX_INTR_THRESHOLD)
                core->intr_thresh_hold++;
        }
    }
  
   if(core->state == CM_STATE_PAUSE || core->state == CM_STATE_HALT) {
        coremu_thread_setpriority(PRIO_PROCESS, core->tid, high_prio);
        pthread_kill(core->thread, COREMU_SIGNAL);
   }    
}

static void adjust_intr_threshold(void)
{
    CMCore* self = coremu_get_core_self();
    self->intr_thresh_hold = 0;
}

event_handler_t event_handler;
void coremu_register_event_handler(event_handler_t fn)
{
    event_handler = fn;
}

event_notifier_t event_notifier;
void coremu_register_event_notifier(event_notifier_t fn)
{
    event_notifier = fn;
}

void coremu_receive_intr()
{
    CMCore *core = coremu_get_core_self();
    while (coremu_intr_p(core)) {
        /* call registed interrupt handler */
        if (event_handler)
            event_handler(coremu_get_intr(core));
    }
}

/* Send an interrupt and notify the accept core
 * Here we use apdaptive signal delay mechanism
 * But this mechanism seems to be wonderful when number of emulated
 * cores is more than 128 (test enviroment R900) */
void coremu_send_intr(void *e, int coreid)
{
    cm_assert(e, "interrupt argument is NULL");
    CMCore *core = coremu_get_core(coreid);

    coremu_put_intr(core, e);

    /* Call event notifier directly if sending interrupt to self.
     * Note that we still need to put the interrupt in the queue, otherwise, the
     * core will lost this interrupt. */
    if (!coremu_hw_thr_p()) {
        if (core == coremu_get_core_self()) {
            event_notifier();
            return;
        }
    }
    coremu_send_signal(core);
}

void coremu_core_signal_handler(int signo, siginfo_t *info, void *context)
{
    CMCore* self = coremu_get_core_self();
    adjust_intr_threshold();
    coremu_thread_setpriority(PRIO_PROCESS, 0, 0);

    if (event_notifier) {
        event_notifier();
    }

    self->sig_pending = 0;
}

