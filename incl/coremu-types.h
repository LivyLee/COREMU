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

/* ************************* Base types ************************* */
#define true 1
#define false 0

typedef void *(thr_start_routine)(void *);

typedef pthread_t core_t;

typedef pid_t tid_t;

typedef pthread_t hw_thr_t;

typedef uint64_t cm_ram_addr_t;        /* XXX: kqemu fails for this? */

/* ************************* Complex types ************************* */

/* Interrupt types */
typedef enum cm_event {
    I8259_EVENT,
    IOAPIC_EVENT,
    IPI_EVENT,
    DIRECT_INTR_EVENT,
    CPU_EXIT_EVENT,
    TB_INVAL_EVENT,
    TB_INVAL_FAST_EVENT,
    CPU_SHUTDOWN_EVENT,
    CM_INTR_CNT,                         /*  #COREMU interrupts */
} cm_event_t;

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

/* interrupt type (device interrupt, IPI, etc.) */
typedef struct cm_intr_s {
    const char *name;                    /* used for debug */
    uint8_t type;                        /* type of the hardware event */

    void *opaque;                        /* emulator specific data */

#if INTR_LOCK
    TAILQ_ENTRY(cm_intr_s) intr;         /* TAILQ entry: one interrupt */
#endif
} cm_intr_t;

/* local timer type */
typedef struct cm_local_alarm_s {
    pthread_t tid;                       /* the thread id */
    int signo;                           /* the signo to send to each thread */
    void *opaque;                        /* qemu_alarm_timer for qemu */
} cm_local_alarm_t;

typedef struct cm_profile_s {
    /* These counter are indexed by cm_event enumeration*/
    uint64_t intr_send[CM_INTR_CNT];     /* number of COREMU interrupts appended in the queue */
    uint64_t intr_recv[CM_INTR_CNT];     /* number of COREMU interrupts received by cores */
    uint64_t *retry_num_addr; 
} cm_profile_t;

typedef struct cm_timer_debug_s {
   uint64_t lapic_timer_cnt;
   uint64_t rearm_cnt;
   uint32_t created_timer;
   uint32_t last_flags;
   uint64_t last_expire_time;
   uint64_t last_current_time;
} cm_timer_debug_t;

/* core tailq type */
TAILQ_HEAD(cores_head, CMCore);
typedef struct cores_head cores_head_t;

/* processor type */
typedef struct CMCore {
    uint32_t serial;                     /* number start from 0 */
    core_t coreid;                       /* ID of the core */
    tid_t tid;                           /* kernel process id */
    TAILQ_ENTRY(CMCore) cores;        /* cores are kept in a tailq */

    cm_local_alarm_t *local_alarm;       /* local alarm timer for each core */

#ifdef INTR_LOCK_FREE
    queue_t *intr_queue;                 /* interrupt queue for the core */
    uint64_t time_stamp;                 /* recode the time of intr pending */
    uint64_t intr_thresh_hold;                /* the thresh hold for intr sending */
    uint8_t sig_pending;                    /* if has signal not receive */
#elif defined(INTR_LOCK)
    TAILQ_HEAD(, cm_intr_s) intr_queue;  /* interrupt queue for the core */
    uint64_t intr_count;                 /* #pending interrupts */
    pthread_mutex_t intr_lock;           /* lock for the interrupt queue */
#else
# error "No HWE queue?"
#endif

    void *opaque;                        /* CPUState of QEMU */
    cm_core_state_t state;               /* state of the CORE */
    cm_profile_t core_profile;           /* CPU profile */
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

    pthread_t  coreid;                   /* core that makes this req */
    int req_id;                          /* for debug */
} cm_ioreq_t;

#endif /* _CM_TYPES_H */

