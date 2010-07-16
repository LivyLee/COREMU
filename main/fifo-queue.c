/*
 * COREMU Parallel Emulator Framework
 *
 * Interfaces for manipulating interrupts in COREMU. (e.g. interrupt from
 * device to core, and IPI between cores).
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
#include <stdbool.h>
#include <signal.h>
#include <assert.h>
#include "fifo-queue.h"
#include "coremu-atomic.h"
#include "coremu-malloc.h"

static void fifo_initialize(lqueue_t *Q)
{
    lnode_t *lnode = new_lnode();                               /* Allocate a free node */
    lnode->next = NULL;                                  
 
    Q->Head = lnode;                                      /* Both Head and Tail point to it */
    Q->Tail = lnode;
    Q->count = 0;
    CM_SPIN_LOCK_INIT(&Q->t_lock);
}

lqueue_t *new_lqueue(void)
{
    lqueue_t *Q = coremu_mallocz(sizeof(lqueue_t));
    fifo_initialize(Q);                                          

    assert(Q->Head != NULL);
    assert(Q->Tail!= NULL);
    assert(Q->Head == Q->Tail);

    return Q;
}

lnode_t *new_lnode(void)
{
    lnode_t *node = coremu_mallocz(sizeof(lnode_t));         
}


/* The interface for queue with lock */
void l_enqueue(lqueue_t *Q, data_type value)
{
    lnode_t *lnode = new_lnode();                              
    lnode->value = value;                                 
    lnode->next = NULL;                                 

    coremu_spin_lock(&Q->t_lock);
    Q->Tail->next = lnode;
    Q->Tail = lnode;
    atomic_incq(&Q->count);
    coremu_spin_unlock(&Q->t_lock);
}

bool l_dequeue(lqueue_t *Q, data_type *value_p)
{
    lnode_t *lnode = Q->Head;
    lnode_t *new_head = lnode->next;
    if (!new_head)
        return false; 
    
    *value_p = new_head->value;
    Q->Head = new_head;
    coremu_free(lnode);
    atomic_decq(&Q->count);
    return true;
}

int64_t fifo_queue_get_size(lqueue_t *Q)
{
    return Q->count;
}

