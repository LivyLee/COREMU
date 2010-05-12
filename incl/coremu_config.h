/* @(#)coremu_config.h
 *
 * This file configures various aspects of COREMU system.
 * Read the comment carefully for each control group.
 *
 */
 
#ifndef _CM_CONFIG_H
#define _CM_CONFIG_H          1

#define CONFIG_COREMU         1         /* whether adapt to coremu */

#define TIMER_THREAD_ENABLE   0         /* use timer thread for per-thread timing? */

/* There are 3 choices for atomic instruction emulation
   for COREMU parallel emulator:
   1. QEMU global 'big' lock (slow, wrong for parallel emulation)
   2. COREMU bus lock (much faster, yet still slow, buggy)
   3. Atomic emulation using CAS1 (currently the best method)

   NOTE: there can only be one TRUE flag for each configuration.

*/

#define USE_QEMU_LOCK         0         /* global lock */
#define USE_BUS_LOCK          0         /* COREMU bus lock support */
#define USE_ATOM_CAS1         1         /* CAS1 support */

#if USE_QEMU_LOCK
# if USE_BUS_LOCK
#  error "global lock has been enable, turn off bus lock or atom impl"
# endif
# if USE_ATOM_CAS1
#  error "global lock has been enable, turn off bus lock or atom impl"
# endif
#else  /* ! USE_QEMU_LOCK */
# if (USE_BUS_LOCK) && (USE_ATOM_CAS1)
#  error "cannot enable both bus lock and atom impl"
# elif (! USE_BUS_LOCK) && (! USE_ATOM_CAS1)
#  error "must choose one among qemu_lock, bus_lock and atom impl"
# endif
#endif

/* There are 2 choices for handling queueing interrupt:
   1. use nonblocking queue
   2. use a tail queue with a big lock

*/

#define INTR_LOCK_FREE      1           /* lock free hardware event queue */
#define INTR_LOCK           0           /* tailq+locking for hardware events */

#if INTR_LOCK_FREE
# if INTR_LOCK
#  error "Lock free interrupt queue enabled. Disable INTR_LOCK"
# endif
#else  /* ! HWE_LOCK_FREE */
# if INTR_LOCK
# else
#  error "MUSTchoose one in INTR_LOCK_FREE and INTR_LOCK"
# endif
#endif

/* There are 2 choices for handling synchrous I/O:
   1. use nonblocking queue to serialize all requests.
   2. only use a lock to serialize request on each processor.
      (a lock for each device?)

*/

#define IOREQ_LOCK_FREE         0      /* sync I/O using lock-free queue */
#define IOREQ_SYNC              1      /* sync I/O using per-core locking */

#if IOREQ_LOCK_FREE
# if IOREQ_SYNC
#  error "lock-free I/O has been enabled, turn off lock-sync I/O"
# endif
#else  /* ! IOREQ_LOCK_FREE */
# if IOREQ_SYNC
# else
#  error "must choose one between IOREQ_LOCK_FREE and IOREQ_SYNC"
# endif
#endif

#if CONFIG_COREMU
#define COREMU_THREAD __thread         /* coremu per-core struct */
#else
#define COREMU_THREAD
#endif

#define COREMU_LAZY_INVAL       0      /* enable lazy invalidate tb */

#define COREMU_PROFILE          0      /* enable profiling for COREMU */
#define COREMU_RETRY_PROFILE    0       /* enable profiling the conflict retry number */      

#endif /* _CM_CONFIG_H */

