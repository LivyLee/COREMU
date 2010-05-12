/* This file defines necessary data structures to implement
   a lock-free FIFO queue using an optimistic approach.

   Which is described in Michael and Scott's
   excellent paper appeared in PODC '96:
   "Simple, Fast, and Practical Non-Blocking and Blocking
   Concurrent Queue Algorithms"

*/
#define DEBUG_COREMU    0
#define VERBOSE_COREMU  0

#include "coremu_utils.h"

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
    /* this is ok */
#else
# include <malloc.h>
#endif

/* Allocate memory aligned to 16 bytes, (see ALIGNMENT
   defined in ms_queue.h) */
static void *ms_allocaligned(size_t size);
static void ms_free(void *ptr);
static void initialize(queue_t *Q);

queue_t *new_queue(void)
{
    /* COREMU XXX: care about qemu_malloc ??? */
    queue_t *Q = ms_allocaligned(sizeof(queue_t));
    initialize(Q);                                           /* Initialize the queue */

    assert(Q->Head.ptr != NULL);
    assert(Q->Tail.ptr != NULL);
    assert(Q->Head.ptr == Q->Tail.ptr);

    return Q;
}

node_t *new_node(void)
{
    /* COREMU XXX: care about qemu_malloc */
    node_t *node = ms_allocaligned(sizeof(node_t));         /* just allocate the node */
    cm_print("new node[%p]", node);
    return node;
}

void enqueue(queue_t *Q, data_type value)
{
    /* -- save the current sigmask */
    sigset_t save_set;
    coremu_sigmask_blk(&save_set, "cannot save mask");

    node_t *node = new_node();                              /* Allocate a new node from the free list */
    node->value = value;                                    /* Copy the enqueued value into node */
    node->next.ptr = NULL;                                  /* Set the next pointer of node to NULL */

    pointer_t tail, next, new_ptr;
    for(;;) {                                               /* Keep trying until Enqueue is done */
        tail = Q->Tail;
        next = tail.ptr->next;

        if(cmp128(&Q->Tail, &tail)) {
            /* Are tail and next consistent? */
            if(next.ptr == NULL) {                          /* Was Tail pointing to the last node */
                new_ptr.ptr = node;                         /* Set the new pointer_t */
                new_ptr.count = next.count + 1;

                if(CAS(&(tail.ptr->next), next, new_ptr)) {   /* Try to link the node at the end of the linked list */
                    break;
                }
            } else {                                        /* Tail was not pointing to the last node */
                new_ptr.ptr = next.ptr;                     /* Set the new pointer_t */
                new_ptr.count = tail.count + 1;
                CAS(&Q->Tail, tail, new_ptr);
            } /* endif */
        } /* endif */
    } /* endloop */

    new_ptr.ptr = node;
    new_ptr.count = tail.count + 1;
    CAS(&Q->Tail, tail, new_ptr);                            /* Enqueue is done. Try to swing Tail to the inserted node */
    atomic_inc64(&Q->count);

    /* -- restore the saved sigmask */
    coremu_sigmask_res(&save_set, "cannot restore mask");
}

int dequeue(queue_t *Q, data_type *pvalue)
{
    /* -- save the current sigmask */
    sigset_t save_set;
    coremu_sigmask_blk(&save_set, "cannot save mask");

    pointer_t head, tail, next, new_ptr;
    for(;;) {                                                /* Keep trying until Dequeue is done */
        head = Q->Head;                                      /* Read Head */
        tail = Q->Tail;                                      /* Read Tail */
        next = head.ptr->next;                               /* Read head.ptr->next */
        if(cmp128(&Q->Head, &head)) {                      /* Are head, tail, next consistent */
            if(head.ptr == tail.ptr) {                       /* Is queue empty or Tail falling behind? */
                if(next.ptr == NULL) {                       /* Is queue empty */
                    /* -- restore the saved sigmask */
                    coremu_sigmask_res(&save_set,
                                       "cannot restore mask");
                    return false;                            /* Queue is empty, couldn't dequeue */
                }
                new_ptr.ptr = next.ptr;                      /* Set new_ptr */
                new_ptr.count = tail.count + 1;
                CAS(&Q->Tail, tail, new_ptr);                /* Tail is falling behind. Try to advance it */
            } else {                                         /* No need to deal with Tail */
                /* Read value before CAS, otherwise another
                   dequeue might free the next node */
                *pvalue = next.ptr->value;
                new_ptr.ptr = next.ptr;
                new_ptr.count = head.count + 1;
                if(CAS(&Q->Head, head, new_ptr)) {           /* Try to swing Head to the next node */
                    break;
                } /* endif */
            } /* endif */
        } /* endif */
    } /* endloop */

    ms_free(head.ptr);                                       /* It is safe to free the old dummy node */
    atomic_dec64(&Q->count);
    coremu_sigmask_res(&save_set, "cannot restore mask");    /* -- restore the saved sigmask */

    return true;                                             /* Queue was not empty, dequeue successed */
}

/* Iterate this queue, and print all values of the nodes */
void print_queue(queue_t *Q)
{
    /* elements of the queue */
    fprintf(stderr, ">>> print queue[%lu] <<<\n",
            Q->count);
    node_t *cur = Q->Head.ptr;
    while(cur != NULL) {
        fprintf(stderr, "cur.value = %lu\n", cur->value);
        cur = cur->next.ptr;
    }
}

uint64_t ms_queue_get_size(queue_t *Q)
{
    return Q->count;
}

/* private functions */
static void *ms_allocaligned(size_t size)
{
    void *ptr;
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
    /* use posix_memalign */
    posix_memalign(&ptr, ALIGNMENT, size);
#else
    /* use obsolete memalign */
    ptr = memalign(ALIGNMENT, size);
#endif

    return ptr;
}

static void ms_free(void *ptr)
{
    /* COREMU XXX: care about qemu_free ??? */
    free(ptr);
}

static void initialize(queue_t *Q)
{
    node_t *node = new_node();                               /* Allocate a free node */
    node->next.ptr = NULL;                                   /* Make it the only node in the linked list */

    Q->Head.ptr = node;                                      /* Both Head and Tail point to it */
    Q->Head.count = nil;                                     /* ??? COREMU: correct ??? */

    Q->Tail.ptr = node;
    Q->Tail.count = nil;

    Q->count = 0;                                            /* no elements initially */
}
