/*
 * COREMU Parallel Emulator Framework
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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "coremu-malloc.h"

static inline void *oom_check(void *ptr)
{
    if (ptr == NULL) {
        abort();
    }
    return ptr;
}

void *cm_malloc(size_t size)
{
    return oom_check(malloc(size));
}

void *cm_mallocz(size_t size)
{
    void *p = cm_malloc(size);
    memset(p, 0, size);
    return p;
}

void cm_free(void *ptr)
{
    free(ptr);
}

