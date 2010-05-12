/* @(#)coremu_utils.h
 *
 * Utilities that only used in COREMU, these
 * utilities may conflict with those defined
 * in QEMU.
 */

#ifndef _COREMU_UTILS_H
#define _COREMU_UTILS_H   1

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/queue.h>

/* Debugging support */
#include <execinfo.h>

/* OS support */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>

/* libtopology suppport */
#include <topology.h>

/* QEMU */
#include <qemu-common.h>

#include "timevar.h"
#include "sched.h"

/* Debugging utilities for COREMU */
#include "coremu.h"
#include "coremu_thread.h"
#include "coremu_atomic.h"
#include "coremu_debug.h"

/* ******** Common utilities ******** */
void maketimeout(struct timespec *tsp, long seconds);
int timeval_subtract(struct timeval *result,
                     struct timeval *x, struct timeval *y);
unsigned long read_host_tsc(void);
int coremu_random(int min, int max);
FILE *coremu_fopen(const char *filename, const char *mode);
void coremu_serialize(void);
pid_t coremu_gettid(void);
void coremu_backtrace(void);

#endif /* _COREMU_UTILS_H */

