/* @(#)tools.h
 *
 * Tools used in COREMU and QEMU (safe to include
 * anywhere). For utilities that are only used in
 * COREMU (e.g. fprintf may conflict with QEMU),
 * refer to 'cm_utils.h'
 *
 * Author: Xi Wu (wuxi@fudan.edu.cn) */

#ifndef _TOOLS_H
#define _TOOLS_H 1

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

/* OS support */
#include <sys/types.h>
#include <sys/syscall.h>

/* QEMU */
#include <sys-queue.h>
#include <qemu-common.h>

/* COREMU */
#include "coremu.h"
#include "coremu_thread.h"

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

#endif /* _TOOLS_H */

