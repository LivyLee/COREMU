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

/* Init a core in COREMU. Basically, coremu core contains
 * a emulator specific data opaque. */
CMCore *coremu_core_init(int id, void *opaque);

/* Run all cores with THR_FN as the start function.
 * The thread function will be passed the opaque object when calling
 * coremu_core_init. */
void coremu_run_all_cores(thr_start_routine thr_fn);

#endif /* _COREMU_CORE_H */
