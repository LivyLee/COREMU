/*
 * COREMU Parallel Emulator Framework
 *
 * This spinlock implementation is from qemu.
 *
 * Copyright (C) 2010 Parallel Processing Institute (PPI), Fudan Univ.
 *  <http://ppi.fudan.edu.cn/system_research_group>
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

#ifndef _COREMU_SPINLOCK_H
#define _COREMU_SPINLOCK_H

typedef unsigned char CMSpinLock;

#define CM_SPIN_LOCK_INITIALIZER { 0 }

#define SPINLOCK_ATTR static __inline __attribute__((always_inline, no_instrument_function))

/* Compile read-write barrier */
#define barrier() asm volatile("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

static inline unsigned short xchg_8(void *ptr, unsigned char x)
{
    __asm__ __volatile__("xchgb %0,%1"
                :"=r" (x)
                :"m" (*(volatile unsigned char *)ptr), "0" (x)
                :"memory");

    return x;
}

#define BUSY 1

SPINLOCK_ATTR void coremu_spin_lock(CMSpinLock *lock)
{
    while (1) {
        if (!xchg_8(lock, BUSY)) return;

        while (*lock) cpu_relax();
    }
}

SPINLOCK_ATTR void coremu_spin_unlock(CMSpinLock *lock)
{
    barrier();
    *lock = 0;
}

SPINLOCK_ATTR int coremu_spin_trylock(CMSpinLock *lock)
{
    return xchg_8(lock, BUSY);
}

#define CM_SPIN_LOCK_INIT(l) coremu_spin_unlock(l)

#endif /* _COREMU_SPINLOCK_H */
