/*
 * COREMU Parallel Emulator Framework
 *
 * This file configures various aspects of COREMU system.
 * Read the comment carefully for each control group.
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

#ifndef _CM_CONFIG_H
#define _CM_CONFIG_H

#define CONFIG_COREMU                   /* whether adapt to coremu */

//#define TIMER_THREAD_ENABLE             /* use timer thread for per-thread timing? */

/* There are 3 choices for atomic instruction emulation
 * for COREMU parallel emulator:
 * 1. QEMU global 'big' lock (slow, wrong for parallel emulation)
 * 2. COREMU bus lock (much faster, yet still slow, buggy)
 * 3. Atomic emulation using CAS1 (currently the best method)
 *
 * NOTE: there can only be one TRUE flag for each configuration.
 */

//#define USE_QEMU_LOCK                   /* global lock */
//#define USE_BUS_LOCK                    /* COREMU bus lock support */
#define USE_ATOM_CAS1                   /* CAS1 support */

#ifdef USE_QEMU_LOCK
# ifdef USE_BUS_LOCK
#  error "global lock has been enable, turn off bus lock or atom impl"
# endif
# ifdef USE_ATOM_CAS1
#  error "global lock has been enable, turn off bus lock or atom impl"
# endif
#else  /* ! USE_QEMU_LOCK */
# if defined(USE_BUS_LOCK) && defined(USE_ATOM_CAS1)
#  error "cannot enable both bus lock and atom impl"
# elif (! defined(USE_BUS_LOCK)) && (! defined(USE_ATOM_CAS1))
#  error "must choose one among qemu_lock, bus_lock and atom impl"
# endif
#endif

/* There are 2 choices for handling queueing interrupt:
 * 1. use nonblocking queue
 * 2. use a tail queue with a big lock
 */

#define INTR_LOCK_FREE                  /* lock free hardware event queue */
//#define INTR_LOCK                       /* tailq+locking for hardware events */

#ifdef INTR_LOCK_FREE
# ifdef INTR_LOCK
#  error "Lock free interrupt queue enabled. Disable INTR_LOCK"
# endif
#else  /* ! HWE_LOCK_FREE */
# ifndef INTR_LOCK
#  error "MUSTchoose one in INTR_LOCK_FREE and INTR_LOCK"
# endif
#endif

/* There are 2 choices for handling synchrous I/O:
 * 1. use nonblocking queue to serialize all requests.
 * 2. only use a lock to serialize request on each processor.
 *    (a lock for each device?)
 */

//#define IOREQ_LOCK_FREE                /* sync I/O using lock-free queue */
#define IOREQ_SYNC                     /* sync I/O using per-core locking */

#ifdef IOREQ_LOCK_FREE
# ifdef IOREQ_SYNC
#  error "lock-free and lock-sync I/O both enabled, only one can be used"
# endif
#else  /* ! IOREQ_LOCK_FREE */
# ifndef IOREQ_SYNC
#  error "must choose one between IOREQ_LOCK_FREE and IOREQ_SYNC"
# endif
#endif

#ifdef CONFIG_COREMU
# define COREMU_THREAD __thread         /* coremu per-core struct */
#else
# define COREMU_THREAD
#endif

//#define COREMU_LAZY_INVAL              /* enable lazy invalidate tb */

//#define COREMU_PROFILE                 /* enable profiling for COREMU */
//#define COREMU_RETRY_PROFILE           /* enable profiling the conflict retry number */

#endif /* _CM_CONFIG_H */
