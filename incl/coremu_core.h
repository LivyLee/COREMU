/* @(#)coremu_core.h
 */

#ifndef _COREMU_CORE_H
#define _COREMU_CORE_H 1

#define COREMU_HLT_TICK   100     /* virtual core sleeps 1 second if HLT executed */

/**
 * Init a core in COREMU. Basically, coremu core contains
 * a emulator specific data OPAQUE. */
cm_core_t *coremu_core_init(void *opaque);

/**
 * Run all cores with THR_FN as the start function, with ARG
 * the parameter passed. */
void coremu_run_all_cores(thr_start_routine thr_fn, void *arg);

/**
 * Get the first core in all cores */
cm_core_t *coremu_get_first_core(void);

/**
 * Return the core data for the core itself */
cm_core_t *coremu_get_self(void);

/**
 * assert the cur thr is a core thr - i.e. its
 * coremu core object is not nil */
void coremu_assert_core_thr(void);

/**
 * Return the specified core object */
cm_core_t *coremu_get_core(pthread_t tid);

/**
 * Return the head of the tailq of cores */
cores_head_t *coremu_get_core_tailq(void);

/**
 * Exit current core with return value pointer VALUE_PTR.
 * @note: the calling thread must NOT be hw thread...  */
void coremu_core_exit(void *value_ptr);

void coremu_pause_core(void);

void coremu_restart_all_cores(void);

void coremu_wait_pause(cm_core_t *core);

/**
 * Wait on a specfic core until it exits... */
int coremu_wait_tid(cm_core_t *coreid, void **val_ptr);

/**
 * Dump the priorities of each CORE thread */
void coremu_dump_prio(void);

/**
 * Dump the profiling information of each CORE */
void coremu_dump_core_profiling_info(void);

/**
 * Dump a specified profile
 */
void coremu_dump_profile(cm_profile_t *profile, const char *msg);

/**
 * Dump debug infomation
 */
void coremu_dump_debug_info(void);

void coremu_dump_timer_debug_info(cm_timer_debug_t * debug_info, const char * msg);

#endif /* _COREMU_CORE_H */

