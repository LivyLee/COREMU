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

#define DEBUG_COREMU    0
#define VERBOSE_COREMU  1

#include <signal.h>
#include "coremu-utils.h"
#include "coremu-hw.h"
#include "coremu-intr.h"
#include "coremu-sched.h"
#include "coremu-core.h"

#define INTR_NUM_THRESHOLD   50    /* notify a CORE if many pending signals */
#define INTR_TIME_THRESHOLD  1000   /* 1ms */
/* hardware profile */
cm_profile_t hw_profile;

/* the flag of adpative signal delay */
int cm_adaptive_intr_delay;

/* the step of intr delay, the signal accept time must be
 * more than cm_intr_delay_step * 10us */
int cm_intr_delay_step;

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

uint64_t coremu_intr_get_size(CMCore *core)
{
    return ms_queue_get_size(core->intr_queue);
}

int coremu_intr_p(CMCore *core)
{
    return coremu_intr_get_size(core) != 0;
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
    while (coremu_intr_p(coremu_get_core_self())) {
        /* call registed interrupt handler */
        if (event_handler)
            event_handler(coremu_get_intr(coremu_get_core_self()));
    }
}

/**
 * Send an interrupt and notify the accept core
 * Here we use apdaptive signal delay mechanism
 * But this mechanism seems to be wonderful when number of emulated
 * cores is more than 128 (test enviroment R900)
 */
void coremu_send_intr(void *e, int coreid)
{
    uint64_t pending_intr;
    CMCore *core = coremu_get_core(coreid);

    coremu_put_intr(core, e);
    pending_intr = coremu_intr_get_size(core);

    /*  here need to do somethings that make the thresh hold to be  more smart!!!*/
    if (core->sig_pending) {
        if (core->intr_thresh_hold < 100)
            core->intr_thresh_hold += 10;
        return;
    }

    if (!cm_adaptive_intr_delay || core->state==STATE_HALT
            || pending_intr > core->intr_thresh_hold
            || (core->state == STATE_PAUSE && pending_intr > 10)) {
        core->sig_pending = 1;
        core->state = STATE_RUN;

        coremu_thread_setpriority(PRIO_PROCESS, core->tid, high_prio);
        pthread_kill(core->coreid, COREMU_SIGNAL);
    }
}

void coremu_cpu_signal_handler(int signo, siginfo_t *info, void *context)
{
    CMCore *core = coremu_get_core_self();

    if (cm_adaptive_intr_delay) {
        uint64_t tsc = read_host_tsc();

        if (core->time_stamp) {
            //if((tsc - core->time_stamp) < ( R900_CFREQ/100000*4))
            if ((tsc - core->time_stamp) <
                    (5000 * (cm_smp_cpus + coremu_get_hostcpu() - 1)/coremu_get_hostcpu())) {
                if (core->intr_thresh_hold < 100)
                    core->intr_thresh_hold += 10;
            } else if ((tsc - core->time_stamp) >
                    (500000 * (cm_smp_cpus + coremu_get_hostcpu() - 1)/coremu_get_hostcpu())) {
                core->intr_thresh_hold = 0;
            } else {
                core->intr_thresh_hold -= 1;
            }
        }

        core->time_stamp = tsc;
    }

    coremu_thread_setpriority(PRIO_PROCESS, 0, 0);

    if (event_notifier) {
        event_notifier();
    }
}

