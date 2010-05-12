/* @(#)coremu_bus_lock.c

   Bus lock abstraction for system emulation: Lock the
   precise memory region accessed by locked instruction.

   NOTE:
   Now this module only works for x86-64 architecture -
   make it aware different architectures.

*/

#define VERBOSE_COREMU  1
#define DEBUG_COREMU    1

#define DEBUG_BUS_LOCK

/* coremu headers */
#include "coremu_utils.h"

/* qemu-specific headers */
#define NEED_CPU_H
#include "target-qemu.h"
#include "coremu_atomic.h"
#include "coremu_bus_lock.h"
#include "coremu_utils.h"
#include "sys/mman.h"

cm_ram_addr_t bitmap_ram_size;      /* size of the bitmap */
uint8_t *bitmap_ram_base;           /* base of the bitmap */

void coremu_bus_lock_init(cm_ram_addr_t phys_ram_size)
{
    bitmap_ram_size = (phys_ram_size >> BITMAP_SHIFT);

#ifdef DEBUG_BUS_LOCK
    cm_print("=== Init bus lock, phy_mem: %lu, map_mem: %lu ===",
             (unsigned long int) phys_ram_size, (unsigned long int) bitmap_ram_size);
#endif

    /* XXX: use anonymous mmap to map memory,
       the intial value is guaranteed to be 0 */
    bitmap_ram_base = mmap(NULL, bitmap_ram_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANON, -1, 0);

    cm_assert((bitmap_ram_base != MAP_FAILED),
              "cannot allocate bus lock bitmap!");

#ifdef DEBUG_BUS_LOCK
    cm_print("=== bus lock is ok ===");
#endif
}

/* Chechk version, use function calls */
#if CHECK_BUS_LOCK

/* Acquire the bus lock for PHYS_ADDR, accessing SIZE bytes memory */
void coremu_bus_lock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    for (;;) {
        uint8_t cur = atomic_compare_exchange8(base, 0, 1);
        if (cur == 0)
            break;
        //sched_yield();
    }
}

/* Release the bus lock for PHYS_ADDR, accessing SIZE bytes memory */
void coremu_bus_unlock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    uint8_t was = atomic_compare_exchange8(base, 1, 0);
    if (was == 0) {
        printf("%p not locked?!\n", base);
        assert(0);
    }
}

/* Try to lock the bus lock for PHY_ADDR, accessing SIZE bytes memory */
uint8_t coremu_bus_trylock(target_phys_addr_t phys_addr, int size)
{
    uint8_t off = phys_addr >> BITMAP_SHIFT;
    uint8_t *base = &bitmap_ram_base[off];

    uint8_t cur = atomic_compare_exchange8(base, 0, 1);
    if (cur == 0)
        return 0;

    return -1;
}

#endif  /* CHECK_BUS_LOCK */
