/* @(#)coremu_hw_lockfree.h

   Use Michal and Scott's well-known nonblocking
   FIFO for synchronized IO requests.
   Refer to "ms_queue.h" for more details.

*/

#ifndef _COREMU_HW_LOCKFREE_H
#define _COREMU_HW_LOCKFREE_H 1

#include "coremu_atomic.h"

extern void coremu_ioreq_ring_init(int smp_cpus);
extern queue_t *Q;

/* shared interface */
static inline uint8_t coremu_ioreq_done(cm_ioreq_t *q)
{
    uint8_t done;
    __asm__ __volatile__("cmpb $0x3,%1\n\t"
                         "sete %0\n\t"
                         : "=a" (done)
                         : "m" (q->state)
                         : "cc");

    return done;
}

static inline void coremu_insert_ioreq(cm_ioreq_t *q)
{
    enqueue(Q, (data_type) q);
}

static inline volatile cm_ioreq_t *coremu_fetch_ioreq(void)
{
    data_type value;
    int ret = dequeue(Q, &value);

    if(ret) {
        return ((volatile cm_ioreq_t *) value);
    } else {
        return NULL;
    }
}

static inline int coremu_ioreq_p(void)
{
    return (ms_queue_get_size(Q) != 0);
}

#endif  /* _COREMU_HW_LOCKFREE_H */

