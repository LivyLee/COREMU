#ifndef _FIFO_QUEUE_H
#define _FIFO_QUEUE_H

#include <malloc.h>
#include <stdbool.h>
#include "coremu-atomic.h"
#include "coremu-spinlock.h"
#define ALIGNMENT  16 

typedef unsigned long data_type;
typedef struct lnode_s lnode_t;
typedef struct lqueue_s lqueue_t;

struct lnode_s {
    lnode_t *next;            /* the next node, together with the tag */
    data_type value;           /* an integer which can hold a pointer */
};

struct lqueue_s {
    lnode_t *Head;            /* head of the queue */
    lnode_t *Tail;            /* tail of the queue */
    uint64_t count;
    CMSpinLock t_lock;
};

lqueue_t *new_lqueue(void);
lnode_t *new_lnode(void);
void l_enqueue(lqueue_t *Q, data_type value);
bool l_dequeue(lqueue_t *Q, data_type *value_p);
uint64_t fifo_queue_get_size(lqueue_t *Q);
#endif

