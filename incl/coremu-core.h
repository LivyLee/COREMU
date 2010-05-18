/*
 * COREMU Parallel Emulator Framework
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

#ifndef _COREMU_CORE_H
#define _COREMU_CORE_H

#define COREMU_HLT_TICK   100     /* virtual core sleeps 1 second if HLT executed */

/**
 * Init a core in COREMU. Basically, coremu core contains
 * a emulator specific data OPAQUE. */
CMCore *coremu_core_init(int id, void *opaque);

/**
 * Run all cores with THR_FN as the start function, with ARG
 * the parameter passed. */
void coremu_run_all_cores(thr_start_routine thr_fn, void *arg);

/* assert the cur thr is a core thr
 * i.e. its  coremu core object is not nil */
void coremu_assert_core_thr(void);

extern int cm_smp_cpus;
/* Return the specified core object. */
CMCore *coremu_get_core(int coreid);

/* Return the core data for the core itself. */
CMCore *coremu_get_core_self();

/* Exit current core with return value pointer VALUE_PTR.
 * NOTE: the calling thread must NOT be hw thread...  */
void coremu_core_exit(void *value_ptr);

void coremu_pause_core(void);

void coremu_restart_all_cores(void);

void coremu_wait_pause(CMCore *core);

/* Wait on a specfic core until it exits... */
int coremu_wait_tid(CMCore *coreid, void **val_ptr);

#endif /* _COREMU_CORE_H */
