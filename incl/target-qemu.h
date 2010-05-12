/* @(#)target-qemu.h
 */

#ifndef _TARGET_QEMU_H
#define _TARGET_QEMU_H 1

#include <config.h>

#ifdef TARGET_X86_64
#define TARGET_LONG_BITS 64
#else
#define TARGET_LONG_BITS 32
#endif

#include <coremu_types.h>
#include <cpu-defs.h>

/* for TB_INVALIDATE_EVENT */
typedef struct qemu_addr_range_s
{
    target_phys_addr_t start;
    target_phys_addr_t end;
    int len;
    int is_cpu_write_access;

} qemu_addr_range_t;

typedef struct qemu_intr_s
{
    int source;             /* the source of the hardware event:
                             * I8259_EVENT: event from the 8259A pic
                             * IOAPIC_EVENT: event from the IOAPIC
                             * LAPIC_EVENT: event from local apic, may be an IPI
                             * CPU_EXIT_EVENT: exit cpu execution
                             * TB_INVALIDATE_EVENT: event to invalidate tb
                             */

    int mask;               /* mask the env mask field */
    int level;              /* triger mode */
    int vector_num;         /* vector number */

    int deliver_mode;
    int triger_mode;
    int target_id;          /* local apic id*/

    /* addr range for TB_INVALIDATE_EVENT */
    qemu_addr_range_t addr_range;

    /* ackowledge for the TB_INVALIDATE_EVENT */
    uint32_t *tb_invalidate_ack_addr;

    /* ram addr for CODE_PROTECT_EVENT */
    unsigned long ram_protect_code;

    /* ackowledge for the CODE_PROTECT_EVENT */
    uint32_t *protect_code_ack_addr;

    /* delayed QEMU hw events */
    TAILQ_ENTRY(qemu_intr_s) qemu_delayed_intr;

} qemu_intr_t;

/* core tailq structure */
TAILQ_HEAD(qemu_intr_head_s, qemu_intr_s);
typedef struct qemu_intr_head_s qemu_intr_head_t;

int is_apic_mmio_addr(int index);

#endif /* _TARGET-QEMU_H */
