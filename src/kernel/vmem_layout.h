#ifndef MEM_LAYOUT_H
#define MEM_LAYOUT_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_HEAP_SIZE ((size_t)0x00100000)

_Static_assert(KERNEL_HEAP_SIZE > 0 && ((KERNEL_HEAP_SIZE & (KERNEL_HEAP_SIZE - 1)) == 0), "KERNEL_HEAP_SIZE must be a power of 2");

/*
 * Kernel virtual memory is aligned like so:
 * 
 * v 0xC0000000      v 0xC0100000               |<---- 1 page ---->|<- KERNEL_HEAP_SIZE ->|
 * +-----------------+-------------+------------+------------------+----------------------+
 * | Area below 1 MB | Kernel code | PMM bitmap | Vmem temp window |     Kernel heap      |
 * +-----------------+-------------+------------+------------------+----------------------+
 */

typedef struct {
	uintptr_t virt_base;
	size_t size;
} vml_runtime_region_t;

void vml_init(void);

vml_runtime_region_t vml_bios_region(void);
vml_runtime_region_t vml_kernel_region(void);
vml_runtime_region_t vml_pmm_bitmap_region(void);
vml_runtime_region_t vml_vmem_temp_window_region(void);
vml_runtime_region_t vml_kernel_heap_region(void);

#endif
