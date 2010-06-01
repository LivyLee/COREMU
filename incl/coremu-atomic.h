/*
 * COREMU Parallel Emulator Framework
 *
 * Atomic support for COREMU system.
 * XXX: Now only support x86-64 architecture.
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

#ifndef _COREMU_ATOMIC_H
#define _COREMU_ATOMIC_H

/************************
 * bit test and set
 ************************/

static inline uint8_t bit_testandset(int *base, int off)
{
    uint8_t readval = 0;

    /* CF <-- Bit(BitBase, BitOffset). */
    __asm__ __volatile__ (
            "lock; btsl %2,%0\n\t"
            "setb %1\n\t"
            : "=m" (*base),"=a" (readval)
            : "Ir" (off)
            : "cc");

    return readval;
}

static inline uint8_t bit_testandreset(int *base, int off)
{
    uint8_t readval = 0;

    /* CF <-- Bit(BitBase, BitOffset). */
    __asm__ __volatile__ (
            "lock; btrl %2,%0\n\t"
            "setb %1\n\t"
            : "=m" (*base),"=a" (readval)
            : "Ir" (off)
            : "cc");

    return readval;
}

static inline uint8_t bit_test(int *base, int off)
{
    uint8_t readval = 0;

    /* CF <-- Bit(BitBase, BitOffset). */
    __asm__ __volatile__ (
            "lock; bt %2,%0\n\t"
            "setb %1\n\t"
            : "=m" (*base),"=a" (readval)
            : "Ir" (off)
            : "cc");

    return readval;
}

/************************
 * exchange
 ************************/

/* swap the value VAL and *p.
 * Return the value swapped out from memory. */

#define EXCHANGE(type) \
    __asm __volatile(                     \
            "lock; xchg##type %1,%2 \n\t" \
            : "=a" (out), "+m" (*p)       \
            : "a" (val)                   \
            )

static inline uint8_t atomic_exchange8(uint8_t *p, uint8_t val)
{
    uint8_t out;
    EXCHANGE("b");
    return out;
}

static inline uint16_t atomic_exchange16(uint16_t *p, uint16_t val)
{
    uint16_t out;
    EXCHANGE("w");

    return out;
}

static inline uint32_t atomic_exchange32(uint32_t *p, uint32_t val)
{
    uint32_t out;
    EXCHANGE("l");

    return out;
}

static inline uint64_t atomic_exchange64(uint64_t *p, uint64_t val)
{
    uint64_t out;
    EXCHANGE("q");

    return out;
}

/************************
 * compare and exchange
 ************************/

/* Atomically compare the value in "p" with "old", and set "p" to "newv"
 * if equal.
 *
 * Return value is the previous value of "p".  So if return value is same
 * as "old", the swap occurred, otherwise it did not. */

#define CMPEXCHANGE(type) \
    __asm__ __volatile__ (              \
            "lock; cmpxchg"type" %2,%1" \
            : "=a" (out), "+m" (*p)     \
            : "q" (newv), "0" (old)     \
            : "cc")

static inline uint8_t atomic_compare_exchange8(uint8_t *p, uint8_t old, uint8_t newv)
{
    uint8_t out;
    CMPEXCHANGE("b");
    return out;
}

static inline uint16_t atomic_compare_exchange16(uint16_t *p, uint16_t old, uint16_t newv)
{
    uint16_t out;
    CMPEXCHANGE("w");
    return out;
}

static inline uint32_t atomic_compare_exchange32(uint32_t *p, uint32_t old, uint32_t newv)
{
    uint32_t out;
    CMPEXCHANGE("l");
    return out;
}

static inline uint64_t atomic_compare_exchange64(uint64_t *p, uint64_t old, uint64_t newv)
{
    uint64_t out;
    CMPEXCHANGE("q");
    return out;
}

static inline uint8_t atomic_compare_exchange16B(uint64_t *memp,
                                                 uint64_t rax, uint64_t rdx,
                                                 uint64_t rbx, uint64_t rcx)
{
    uint8_t z;
    __asm __volatile__ ( "lock; cmpxchg16b %3\n\t"
                         "setz %2\n\t"
                         : "=a" (rax), "=d" (rdx), "=r" (z), "+m" (*memp)
                         : "a" (rax), "d" (rdx), "b" (rbx), "c" (rcx)
                         : "memory", "cc" );
    return z;
}


/************************
 * atomic inc
 ************************/

#define INC(type) \
    __asm__ __volatile__(   \
            "lock; inc"type" %0" \
            : "+m" (*p)     \
            :               \
            : "cc")

static __inline__ void atomic_inc16(uint16_t *p)
{
    INC("w");
}

static __inline__ void atomic_inc32(uint32_t *p)
{
    INC("l");
}

static __inline__ void atomic_inc64(uint64_t *p)
{
    INC("q");
}

/************************
 * atomic dec
 ************************/

#define DEC(type) \
    __asm__ __volatile__(   \
            "lock; dec"type" %0" \
            : "+m" (*p)     \
            :               \
            : "cc")

static __inline__ void atomic_dec16(uint16_t *p)
{
    DEC("w");
}

static __inline__ void atomic_dec32(uint32_t *p)
{
    DEC("l");
}

static __inline__ void atomic_dec64(uint64_t *p)
{
    DEC("q");
}

/* Memory Barriers: x86-64 ONLY now */
#define mb()    asm volatile("mfence":::"memory")
#define rmb()   asm volatile("lfence":::"memory")
#define wmb()   asm volatile("sfence" ::: "memory")

#endif /* _COREMU_ATOMIC_H */

