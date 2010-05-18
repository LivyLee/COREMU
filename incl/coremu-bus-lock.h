/*
 * COREMU Parallel Emulator Framework
 *
 * Bus lock abstraction for functional emulation: lock the
 * precise memory region accessed by locked instruction.
 *
 * According to "Intel System Programming Manual, Vol.- 3,
 * 7.1.2.2". lock insn locks the area of memory defined.
 *
 *  XXX: lock the 64-bit aligned region, Any better idea?
 *
 *  XXX: BUS Lock is now an idea WORSE than atomically
 *       emulating ocked instructions using CAS1.
 *
 * Refer to coremu-config.h for more DETAILS.
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

#ifndef _COREMU_BUS_LOCK_H
#define _COREMU_BUS_LOCK_H

#include <coremu-config.h>
#include <coremu-atomic.h>
#include <sched.h>

#define CHECK_BUS_LOCK     0    /* function call, no inline */
#define BITMAP_SHIFT       3    /* bitmap calculate */

/* exported states */
extern cm_ram_addr_t bitmap_ram_size;
extern uint8_t *bitmap_ram_base;

/* INIT routine.
   XXX: must call after QEMU memory initialized */
void coremu_bus_lock_init(cm_ram_addr_t phys_ram_size);

/* BUS lock interfaces */
#if CHECK_BUS_LOCK
void coremu_bus_lock(target_phys_addr_t phys_addr, int size);
void coremu_bus_unlock(target_phys_addr_t phys_addr, int size);
uint8_t coremu_bus_trylock(target_phys_addr_t phys_addr, int size);
#else  /* ! CHECK_BUS_LOCK */

/* Acquire the bus lock for PHYS_ADDR, accessing SIZE bytes memory */
static inline void coremu_bus_lock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    for (;;) {
        uint8_t cur = atomic_compare_exchange8(base, 0, 1);
        if (cur == 0)
            break;
        //sched_yield();
    }
}

/* Release the bus lock for PHYS_ADDR, accessing SIZE bytes memory */
static inline void coremu_bus_unlock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    uint8_t was = atomic_compare_exchange8(base, 1, 0);
    if (was == 0) {
        printf("%p not locked?!\n", base);
        assert(0);
    }
}

/* Try to lock the bus lock for PHY_ADDR, accessing SIZE bytes memory */
static inline uint8_t coremu_bus_trylock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    uint8_t cur = atomic_compare_exchange8(base, 0, 1);
    if (cur == 0)
        return 0;

    return -1;
}

#endif /* CHECK_BUS_LOCK */

#endif /* _COREMU_BUS_LOCK_H */
