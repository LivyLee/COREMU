/* @(#)coremu_sched.h
 *
 * Scheduling support for COREMU parallel emulation library
 *
 */
#ifndef _COREMU_SCHED_H
#define _COREMU_SCHED_H 1

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

