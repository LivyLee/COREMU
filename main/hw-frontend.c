/*
 * COREMU Parallel Emulator Framework
 *
 * Frontend of the coremu hw emulation layer.
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

#define _GNU_SOURCE

/*#define DEBUG_COREMU*/
#define VERBOSE_COREMU

#include "utils.h"
#include "coremu-hw.h"
#include "coremu-sched.h"
#include "core.h"

#if IOREQ_LOCK_FREE
#include <coremu_hw_lockfree.h>
#endif

/* === Interfaces for IO requests manipulation === */
void coremu_prepare_pio_ioreq(cm_ioreq_t *q, uint64_t addr,
                              uint64_t val, uint64_t size,uint8_t dir)
{
    /* init normal fields */
    q->addr = addr;
    q->data = val;
    q->size = size;
    q->dir = dir;

    q->state = STATE_IOREQ_READY;
    q->type = IOREQ_TYPE_PIO;

    CMCore *core = coremu_get_core_self();
    q->core_thread = core->thread;

    static int id = 0;
    q->req_id = id++;
}

void coremu_prepare_mmio_ioreq(cm_ioreq_t *q,uint64_t addr,
                               uint64_t val, uint32_t shift,
                               uint32_t index, uint8_t dir)
{
    /* init normal fields */
    q->addr = addr;
    q->data = val;
    q->shift = shift;
    q->index = index;
    q->dir = dir;

    q->state = STATE_IOREQ_READY;
    q->type = IOREQ_TYPE_MMIO;

    CMCore *core = coremu_get_core_self();
    q->core_thread = core->thread;

    static int id = 0;
    q->req_id = id++;
}

/* ******** ISA support ******** */
#ifdef IOREQ_SYNC                  /* synchronized I/O */

/* -- Port Mapped I/O -- */
void coremu_cpu_out(int addr, int val, uint64_t size)
{
    ioport_write(size, addr, val);
}

int coremu_cpu_in(int addr, uint64_t size)
{
    int ret;
    ret = ioport_read(size, addr);
    return ret;
}

/* -- Memory Mapped I/O -- */
void coremu_mmio_write(int index, int val, uint32_t shift, uint64_t addr)
{
    do_mmio_write(index, val, shift, addr);
}

int coremu_mmio_read(int index, uint32_t shift, uint64_t addr)
{
    int ret;
    ret = do_mmio_read(index, shift, addr);
    return ret;
}

#elif defined(IOREQ_LOCK_FREE)           /* lockfree I/O */

static inline cm_ioreq_t *ioreq_alloc(void);
static inline void ioreq_free(cm_ioreq_t *q);

/* -- Port Mapped IO -- */
void coremu_cpu_out(int addr, int val, uint64_t size)
{
    /* step 0: scheduling feedback */
    //coremu_sched(EVENT_IO);

    /* step 1: prepare request */
    cm_ioreq_t *q = ioreq_alloc();
    coremu_prepare_pio_ioreq(q, addr, val, size, IOREQ_WRITE);

    /* step 2: send and wait */
    coremu_insert_ioreq(q);
    for(;;) {
        if(coremu_ioreq_done(q))
            break;
    }

    /* step 3: free req */
    ioreq_free(q);
}

int coremu_cpu_in(int addr, uint64_t size)
{
    int ret;

    /* step 0: scheduling feedback */
    //coremu_sched(EVENT_IO);

    /* step 1: prepare request */
    cm_ioreq_t *q = ioreq_alloc();
    coremu_prepare_pio_ioreq(q, addr, 0, size, IOREQ_READ);

    /* step 2: send and wait */
    coremu_insert_ioreq(q);
    for(;;) {
        if(coremu_ioreq_done(q))
            break;
    }

    /* step 3: get value and free req */
    ret = q->data;
    ioreq_free(q);

    /* step 4: return the value read */
    return ret;
}

/* -- Memory mapped IO -- */
void coremu_mmio_write(int index, int val, uint32_t shift, uint64_t addr)
{
    /* step 0: scheduling feedback */
    //coremu_sched(EVENT_IO);

    /* step 1: prepare request */
    cm_ioreq_t *q = ioreq_alloc();
    coremu_prepare_mmio_ioreq(q, addr, val, shift,
                              index, IOREQ_WRITE);

    /* step 2: send and wait */
    coremu_insert_ioreq(q);
    for(;;) {
        if(coremu_ioreq_done(q))
            break;
    }

    /* step 3: free req */
    ioreq_free(q);
}

int coremu_mmio_read(int index, uint32_t shift, uint64_t addr)
{
    int ret;

    /* step 0: scheduling feedback */
    //coremu_sched(EVENT_IO);

    /* step 1: prepare request */
    cm_ioreq_t *q = ioreq_alloc();
    coremu_prepare_mmio_ioreq(q, addr, 0, shift,
                              index, IOREQ_READ);

    /* step 2: send and wait */
    coremu_insert_ioreq(q);
    for(;;) {
        if(coremu_ioreq_done(q))
            break;
    }

    /* step 3: get value and free req */
    ret = q->data;
    ioreq_free(q);

    /* step 4: return the value read */
    return ret;
}

static inline cm_ioreq_t *ioreq_alloc(void)
{
    return qemu_mallocz(sizeof(cm_ioreq_t));
}

static inline void ioreq_free(cm_ioreq_t *q)
{
    qemu_free(q);
}

#else

#error "discard tailq version I/O..."

#endif /* swith I/O mode */
