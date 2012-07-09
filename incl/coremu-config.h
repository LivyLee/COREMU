/*
 * COREMU Parallel Emulator Framework
 *
 * This file configures various aspects of COREMU system.
 * Read the comment carefully for each control group.
 *
 * Copyright (C) 2010 Parallel Processing Institute (PPI), Fudan Univ.
 *  <http://ppi.fudan.edu.cn/system_research_group>
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

#define COREMU_MAX_CPU 255              /* the max cpu supported by coremu */

/* Signals used in COREMU */
#define TIMERRTSIG              SIGRTMIN          /* Timer thread implicit signal */
#define COREMU_CORE_ALARM       (SIGRTMIN + 1)    /* CORE alarm signal */
#define COREMU_HARDWARE_ALARM   (SIGRTMIN + 2)    /* Hardware alarm signal */
#define COREMU_SIGNAL           (SIGRTMIN + 3)    /* COREMU Interrupts */
#define COREMU_AIO_SIG          (SIGRTMIN + 4)    /* Aynchrous I/O (e.g. DMA) */

#ifdef CONFIG_COREMU
# define COREMU_THREAD __thread         /* coremu per-core struct */
#else
# define COREMU_THREAD
#endif

#include "coremu-feature.h"

/* Test the performance of using function to access memory. */
//#define MEMOP_AS_FUNC

/* Flags for replay */
#define CONFIG_REPLAY

#ifdef CONFIG_REPLAY

#define USE_RWLOCK
#define CONFIG_MEM_ORDER

// Don't acquire lock when recording memory order.
// Used to test the performance impact of atomic instructions.
#define NO_LOCK

//#  define TLBFLUSH_AS_INTERRUPT

//#define DEBUG_REPLAY
#ifdef DEBUG_REPLAY
//#  define ADD_TLBFLUSH
//#  define DEBUG_MEM_ACCESS
//#  define CHECK_MEMOP_CNT
#  define ASSERT_REPLAY_PC
//#  define ASSERT_REPLAY_TLBFILL
//#  define ASSERT_REPLAY_TBFLUSH
//#  define ASSERT_REPLAY_TLBFLUSH
//#  define ASSERT_REPLAY_GENCODE
//#  define ASSERT_REPLAY_TB_PHYS_HASH
//#  define ASSERT_REPLAY_TB_JMP_HASH
//#  define ASSERT_REPLAY_TB_JMP_CACHE_MISS_CNT
#endif

/* Use log buffer  */
//#define REPLAY_LOGBUF

//#define SLOW_HOST2RAMADDR

#endif /* CONFIG_REPLAY */

#endif /* _CM_CONFIG_H */

