/* @(#)coremu_intr.h
 */

#ifndef _COREMU_INTR_H
#define _COREMU_INTR_H 1

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

