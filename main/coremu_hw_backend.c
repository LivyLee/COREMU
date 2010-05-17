/*
 * COREMU Parallel Emulator Framework
 *
 * Backend of the coremu hw emulation layer. 
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

/*#define DEBUG_COREMU*/
/*#define DEBUG_IOREQ_WAIT*/
/*#define DEBUG_IOREQ_HANDLER*/
/*#define DEBUG_IOREQ_QUEUE*/
#define VERBOSE_COREMU
/*#define DEBUG_IO_REQ*/

#include "coremu_utils.h"
#include "coremu_hw.h"
#include "coremu_sched.h"

#ifdef IOREQ_SYNC                  /* lock-synchronized I/O */

void coremu_ioreq_wait()
{
    /* just a place-holder, do nothing */
    assert(0);
}

#elif defined(IOREQ_LOCK_FREE)

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
#ifdef DEBUG_IOREQ_HANDLER
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
