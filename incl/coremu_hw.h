/* @(#)coremu_hw.h
 *
 * Basic hardware abstractions for COREMU.
 *
 */

#ifndef _COREMU_HW_H
#define _COREMU_HW_H 1

#define HWEVENT_TYPE_TIMER      0

/* REQ type */
#define IOREQ_TYPE_PIO          0
#define IOREQ_TYPE_MMIO         1

/* REQ dir */
#define IOREQ_READ              0
#define IOREQ_WRITE             1

/* REQ status */
#define STATE_IOREQ_NONE        0
#define STATE_IOREQ_READY       1
#define STATE_IOREQ_INPROCESS   2
#define STATE_IOREQ_DONE        3

/* IOREQ 2^N bytes */
#define IOREQ_BYTE              0
#define IOREQ_WORD              1
#define IOREQ_LONG              2

/* ******** hardware profile ******** */
extern cm_profile_t hw_profile;

/* ******** External states ******** */
void coremu_init_hw(int smp_cpus);
void coremu_signal_hw_thr(int signo);
hw_thr_t coremu_get_hw_id(void);
int coremu_hw_thr_p(void);
void coremu_assert_hw_thr(const char *msg);
void coremu_assert_not_hw_thr(const char *msg);

/* ******** COREMU frontend interfaces ******** */
void coremu_prepare_pio_ioreq(cm_ioreq_t *q, uint64_t addr,
                              uint64_t val, uint64_t size, uint8_t dir);

void coremu_prepare_mmio_ioreq(cm_ioreq_t *q, uint64_t addr,
                               uint64_t val, uint32_t shift,
                               uint32_t index, uint8_t dir);

void coremu_cpu_out(int addr, int val, uint64_t size);
int coremu_cpu_in(int addr, uint64_t size);
void coremu_mmio_write(int index, int val, uint32_t shift, uint64_t addr);
int coremu_mmio_read(int index, uint32_t shift, uint64_t addr);

/* ******** COREMU hardware backend interfaces ******** */
void coremu_ioreq_wait(void);
int coremu_ioreq_exit_p(void);
void coremu_inc_ioreq_exit(void);
void coremu_dec_ioreq_exit(void);

/* ******** QEMU I/O interfaces ******** */
extern uint32_t ioport_read(int index, uint32_t address);

extern void ioport_write(int index,
                         uint32_t address, uint32_t data);

extern uint32_t do_mmio_read(uint32_t index,
                             uint32_t shift, uint64_t addr);

extern void do_mmio_write(uint32_t index, uint64_t val,
                          uint32_t shift, uint64_t addr);

#endif /* _COREMU_HW_H */

