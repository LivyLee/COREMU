/* This file defines necessary data structures to implement
   a lock-free FIFO queue using an optimistic approach.

   Which is described in Michael and Scott's
   excellent paper appeared in PODC '96:
   "Simple, Fast, and Practical Non-Blocking and Blocking
   Concurrent Queue Algorithms"

*/
#ifndef _MS_QUEUE_H
#define _MS_QUEUE_H 1

#include "coremu_atomic.h"

typedef uint64_t data_type;
typedef struct node_s node_t;
typedef struct pointer_s pointer_t;
typedef struct queue_s queue_t;

struct pointer_s {
    node_t *ptr;               /* lower order: (node_t *) */
    uint64_t count;            /* higher order: unsigned integer */
};

struct node_s {
    pointer_t next;            /* the next node, together with the tag */
    data_type value;           /* now a simple 64-bit int */
};

struct queue_s {
    pointer_t Head;            /* head of the queue */
    pointer_t Tail;            /* tail of the queue */
    uint64_t count;            /* count the number of elements */
};

/* macros */
#define nil        0           /* predefined tag */
#define ALIGNMENT  16          /* all data must be aligned to 16 bytes */

static inline uint8_t cmp128(pointer_t *p1, pointer_t *p2);
static inline uint64_t CAS(pointer_t *ptr, pointer_t old, pointer_t new);

/* Sematics of this function, comparing two 128 bits objects:
   if(p1 == p2)
      return 1;
   else
      return 0;

*/
static inline uint8_t cmp128(pointer_t *p1, pointer_t *p2)
{
    if(CAS(p1, *p2, *p1)) {
        return 1;
    } else {
        return 0;
    }
}

/* Atomic compare and swap for the pointer_t structure.
   Semantics of this function:
   (rdx : rax) <- old;
   (rcx : rbx) <- new;
   if((rdx : rax) == *ptr) {
      *ptr <- (rcx : rbx);
      ZF <- 1;
   } else {
      (rdx :rax) <- *ptr;
      ZF <- 0;
   }

*/
static inline uint64_t CAS(pointer_t *ptr, pointer_t old, pointer_t new)
{
    volatile uint64_t rdx, rax;
    volatile uint64_t rcx, rbx;

    rax = (uint64_t) old.ptr;   /* lower  order 64 bits */
    rdx = old.count;            /* higher order 64 bits */

    rbx = (uint64_t) new.ptr;   /* lower  order 64 bits */
    rcx = new.count;            /* higher order 64 bits */

    assert(((uint64_t)ptr & 0xf) == 0);

    return atomic_compare_exchange16B((uint64_t *) ptr,
                                      rax, rdx, rbx, rcx);
}

/* function prototypes */
queue_t *new_queue(void);
node_t *new_node(void);
void enqueue(queue_t *q, data_type val);
int dequeue(queue_t *q, data_type *pvalue);
void print_queue(queue_t *Q);
uint64_t ms_queue_get_size(queue_t *Q);

#endif /* _MS_QUEUE_H */
