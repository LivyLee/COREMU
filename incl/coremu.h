/* @(#)coremu.h
 */

#ifndef _COREMU_H
#define _COREMU_H 1

#include "coremu_config.h"
#include "ms_queue.h"
#include "coremu_types.h"
#include "coremu_core.h"

/* Signals used in COREMU */
#define TIMERRTSIG              SIGRTMIN          /* Timer thread implicit signal */
#define COREMU_TIMER_SIGNAL     (SIGRTMIN + 1)    /* CORE alarm signal */
#define COREMU_TIMER_ALARM      (SIGRTMIN + 2)    /* Hardware alarm signal */
#define COREMU_SIGNAL           (SIGRTMIN + 3)    /* COREMU Interrupts */
#define COREMU_AIO_SIG          (SIGRTMIN+4)      /* Aynchrous I/O (e.g. DMA) */

extern int cm_profiling_p;

extern __thread unsigned long int cm_retry_num;
/**
 * Init the whole COREMU library for multicore
 * system emulation.
 *
 * @smp_cpus: number of cores.
 * @msg_fn: COREMU signal handler in guest
 *          system emulator. */
void coremu_init(int smp_cpus, msg_handler msg_fn);

/**
 * Whether COREMU has started all the cores to run. */
int coremu_init_done_p(void);

/**
 * Core threads wait until the COREMU intialization
 * has done...  */
void coremu_wait_init(void);

#endif /* _COREMU_H */

