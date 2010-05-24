/*
 * COREMU Parallel Emulator Framework
 *
 * This file defines the types in COREMU system
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

#ifndef _CM_TYPES_H
#define _CM_TYPES_H

#include "coremu-config.h"
#include "ms-queue.h"

/* ************************* Base types ************************* */
#define true 1
#define false 0

typedef void *(thr_start_routine)(void *);

typedef pid_t tid_t;

typedef pthread_t hw_thr_t;

typedef uint64_t cm_ram_addr_t;        /* XXX: kqemu fails for this? */

/* ************************* Complex types ************************* */

/* State of COREMU CORE */
typedef enum cm_core_state {
    STATE_RUN,                           /* NOT halt */
    STATE_HALT,                          /* Halted state (e.g. execute HLT insn) */
    STATE_PAUSE,
} cm_core_state_t;

/* CORE scheduling type */
typedef enum sched_event {
    EVENT_HALTED,
    EVENT_CPU,
    EVENT_IO,
    EVENT_CNT,
} sched_event_t;

/* Hardware scheduleing type */
typedef enum sched_hw_event {
    EVENT_NO_IOREQ,
    EVENT_IOREQ,
    EVENT_HW_CNT,
} sched_hw_event_t;

typedef struct cm_timer_debug_s {
   uint64_t lapic_timer_cnt;
   uint64_t rearm_cnt;
   uint32_t created_timer;
   uint32_t last_flags;
   uint64_t last_expire_time;
   uint64_t last_current_time;
} cm_timer_debug_t;

/* processor type */
typedef struct CMCore {
    uint32_t serial;                     /* number start from 0 */
    pthread_t thread;                    /* ID of the core */
    tid_t tid;                           /* kernel process id */

    queue_t *intr_queue;                 /* interrupt queue for the core */
    uint64_t time_stamp;                 /* recode the time of intr pending */
    uint64_t intr_thresh_hold;           /* the thresh hold for intr sending */
    uint8_t sig_pending;                 /* if has signal not receive */

    void *opaque;                        /* CPUState of QEMU */
    cm_core_state_t state;               /* state of the CORE */
    cm_timer_debug_t debug_info;         /* Used for timer debug */
} CMCore;

typedef struct cm_ioreq_s {
    uint64_t addr;                       /* physical address */
    uint64_t size;                       /* size in bytes  */
    uint64_t data;                       /* data (or paddr of data) */
    uint8_t state;                       /* io handling status */
    uint8_t data_is_ptr;                 /* if 1, data above is the guest
                                             paddr of the real data to use. */

    uint32_t index;                      /* only for mmio request */
    uint32_t shift;                      /* only for mmio request */

    uint8_t dir;                         /* direction, 0 = r, 1 = w */
    uint8_t type;                        /* I/O type */
    uint64_t io_count;                   /* How many IO done on a vcpu */

    pthread_t  core_thread;              /* core that makes this req */
    int req_id;                          /* for debug */
} cm_ioreq_t;

#endif /* _CM_TYPES_H */

