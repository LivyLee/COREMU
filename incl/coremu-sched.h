/*
 * COREMU Parallel Emulator Framework
 *
 * Scheduling support for COREMU parallel emulation library
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

#ifndef _COREMU_SCHED_H
#define _COREMU_SCHED_H

/* system headers */
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

/* controlling scheduling */
#define CM_ENABLE_SCHED  0

#define CM_ENABLE_BIND_CORE 1

/* bind the vcpu to physical cpu */
#define CM_BIND_SAME_CORE 1

/* bind the near vcpus to the same physcal cpu */
#define CM_BIND_SAME_CORE2 0

#define CM_BIND_SOCKET 0

/* halt method: nanosleep, sched_yield or hybrid */
#define HALT_NANOSLEEP  1
#define HALT_YIELD      0
#define HALT_HYBRID     0

/* scheduling states */
#define STATE_CPU        0
#define STATE_IO         1
#define STATE_HALTED     2
#define STATE_CNT        3

/* policies */
#define CM_SCHED_POLICY  (SCHED_OTHER)
#define CM_SCHED_INHERIT (PTHREAD_INHERIT_SCHED)

/* thresholds */
#define CM_NO_IO_LIMIT  5
#define CM_BONUS_IO_CNT 100     /* give bonus for 100 times in STATE_IO */
#define CM_PRIO_CPU_CNT 100
#define CM_HW_IO_CNT    1000    /* give hw high prio if performed many io reqs */

extern int min_prio, max_prio;
extern int low_prio, avg_prio, high_prio;

/* init the scheduling support */
void coremu_init_sched_all(void);
void coremu_init_sched_core(void);

int coremu_get_hostcpu(void);
int coremu_get_maxprio(void);
int coremu_get_minprio(void);

/* schedule core threads */
void coremu_sched(sched_event_t e);

/* schedule hw thread */
void coremu_hw_sched(sched_hw_event_t e);

#endif /* _COREMU_SCHED_H */
