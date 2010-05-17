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

#ifndef _COREMU_INTR_H
#define _COREMU_INTR_H

/**
 * Insert an intrrupt into queue. */
void coremu_put_intr(void *e, size_t size, CMCore *core);

/**
 * Get the first interrupt from queue. */
void *coremu_get_intr(CMCore *core);

/**
 * Get the number of pending interrupts*/
uint64_t coremu_intr_get_size(CMCore *core);

/**
 * Does the CORE has pending interrupt? */
int coremu_intr_p(CMCore *core);

/**
 * Prepare a standard 'intr' */
void coremu_prepare_intr(qemu_intr_t *intr,
                         int source, int level,
                         int mask, int vector_num,
                         int triger_mode, int deliver_mode, int target_id);

/**
 * Prepare a TB invalidate 'intr'
 */
void coremu_prepare_tbinval_intr(qemu_intr_t *intr,
                                 int source,
                                 target_phys_addr_t start,
                                 target_phys_addr_t end,
                                 int len, int is_cpu_write_access,
                                 uint32_t *tb_invalidate_ack);
/**
 * Notify the core of the interrupt. */
void coremu_notify_intr(void *e, size_t size, CMCore *core);

/**
 * Broadcast an interrupt to all cores.
 *
 * XXX: done by 'pthread_kill' for each core.
 * Any better idea? */
void coremu_broadcast_intr(void *e, size_t size);

/**
 * Broadcast an interrupt to all cores
 * except the calling core.
 */
void coremu_broadcast_intr_other(void *e, size_t size);

#endif /* _COREMU_INTR_H */

