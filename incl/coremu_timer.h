#ifndef COREMU_TIMER_H
#define COREMU_TIMER_H

#include <time.h>

/* create a per-core local alarm timer */
int coremu_local_timer_create(int signo,
                              void *opaque,
                              timer_t *host_timer,
                              cm_local_alarm_t **lalarm,
                              pthread_t thrid);

/* start the coremu timer thread */
void coremu_start_timer_thread(void);

#endif
