/* @(#)coremu_thread.h
 *
 * Thread abstraction for COREMU
 *
 */

#ifndef _CM_THREAD_H
#define _CM_THREAD_H 1

#include <sys/time.h>
#include <sys/resource.h>
#include "pthread.h"

/* error-checking lock for static lock */
#define COREMU_LOCK_INITIALIZER  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#define COREMU_COND_INITIALIZER PTHREAD_COND_INITIALIZER

//extern __thread sigset_t cm_blk_sigset;

/* mutex related interfaces */
static inline void coremu_mutex_lock(pthread_mutex_t *mutex,
                                     const char *errmsg)
{
    int err = pthread_mutex_lock(mutex);
    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
}

static inline void coremu_mutex_unlock(pthread_mutex_t *mutex,
                                       const char *errmsg)
{
    int err = pthread_mutex_unlock(mutex);
    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
}

static inline void coremu_mutex_destroy(pthread_mutex_t *mutex,
                                        const char *errmsg)
{
    int err = pthread_mutex_destroy(mutex);
    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
}

/* conditional related interfaces */
static inline int coremu_cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock)
{
    return pthread_cond_wait(cond, lock);
}

static inline int coremu_cond_signal(pthread_cond_t *cond)
{
    return pthread_cond_signal(cond);
}

static inline int coremu_cond_broadcast(pthread_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

static inline void coremu_cond_destroy(pthread_cond_t *cond,
                                      const char *errmsg)
{
    int err = pthread_cond_destroy(cond);
    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
}

/* ******** Pthread signal functionalities ******** */

static inline void coremu_sigmask_blk(sigset_t *oldset,
                                      const char *errmsg)
{
#if 1
    sigset_t set;
    sigfillset(&set);
    int err = pthread_sigmask(SIG_BLOCK, &set, oldset);

    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
#endif
}

/* restore the signals from SET */
static inline void coremu_sigmask_res(sigset_t *set,
                                      const char *errmsg)
{
#if 1
    int err = pthread_sigmask(SIG_SETMASK, set, NULL);

    if(err) {
        printf("%s\n", errmsg);
        assert(0);
    }
#endif
}

/* Thread scheduling */
static inline int coremu_thread_setpriority(int which, int who, int prio)
{
    int err = setpriority(which, who, prio);
    assert(! err);
    return true;
}

static inline int coremu_thread_getpriority(int which, int who)
{
    return getpriority(which, who);
}

#endif /* _CM_THREAD_H */

