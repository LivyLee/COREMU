/* @(#)coremu_hw_backend.c
   backend of the coremu hw emulation layer. */

#define DEBUG_COREMU          0
#define DEBUG_IOREQ_WAIT      0
#define DEBUG_IOREQ_HANDLER   0
#define DEBUG_IOREQ_QUEUE     0
#define VERBOSE_COREMU        1
#define DEBUG_IO_REQ          0

#include "coremu_utils.h"
#include "coremu_hw.h"
#include "coremu_sched.h"

#if IOREQ_SYNC                  /* lock-synchronized I/O */

void coremu_ioreq_wait()
{
    /* just a place-holder, do nothing */
    assert(0);
}

#elif IOREQ_LOCK_FREE

#include <coremu_hw_lockfree.h>

#define IOWAIT_TIME           (2)
#define NOREQ_THRESHOLD       (100000000)

static inline void ioreq_handler(volatile cm_ioreq_t *req);
static volatile int noreq_cnt = 0;

/* Use lock-free queue to handle synchornized I/O requests.
   Refer to "coremu_hw_lockfree.h" for more details. */
void coremu_ioreq_wait()
{
    coremu_assert_hw_thr("ONLY hw thread can wait for ioreq!");

    volatile cm_ioreq_t *current_ioreq;
    while((! coremu_ioreq_exit_p()))
    {
        /* XXX: we dec the priority of the hw thread,
           if got long "time" with no request.
           -- Any better IDEA? -- */
        if(! coremu_ioreq_p()) {
            if(noreq_cnt == NOREQ_THRESHOLD) {
                coremu_hw_sched(EVENT_NO_IOREQ);
            } else {
                noreq_cnt++;
            }
            continue;
        } else if(noreq_cnt != 0) {
            noreq_cnt = 0;
        }

        coremu_hw_sched(EVENT_IOREQ);
        current_ioreq = coremu_fetch_ioreq();
        ioreq_handler(current_ioreq);
        current_ioreq->state = STATE_IOREQ_DONE;
    }
}

static inline void ioreq_handler(volatile cm_ioreq_t *req)
{
    switch(req->type)
    {
    case IOREQ_TYPE_PIO:
    {
#if DEBUG_IOREQ_HANDLER
        cm_print("handle phys I/O: %d",  req->req_id);
#endif

        if(req->dir == IOREQ_READ) {
            req->data = ioport_read(req->size, req->addr);
        } else if(req->dir == IOREQ_WRITE) {
            ioport_write(req->size, req->addr, req->data);
        } else {
            cm_assert(false, "unknown PIO dired");
        }

        break;
    }
    case IOREQ_TYPE_MMIO:
    {
#if DEBUG_IOREQ_HANDLER
        cm_print("handle mmio-req: %d", req->req_id);
#endif

        if(req->dir == IOREQ_READ) {
            req->data = do_mmio_read(req->index, req->shift, req->addr);
        } else if(req->dir == IOREQ_WRITE) {
            do_mmio_write(req->index, req->data, req->shift, req->addr);
        } else {
            cm_assert(false, "unknown MMIO dired");
        }

        break;
    }
    default:
        cm_assert(false, "UNKNOWN req type");
    }
}

#else

#error "unknown I/O mode"

#endif  /* switch I/O mode */
