#include <stdbool.h>
#include <stddef.h>
#include "vmem.h"
#include "pmm.h"
#include "memory_helpers.h"
#include "screen/panic.h"
#include "screen/stdio.h"
#include "kernel_stdlib.h"
#include "vmem_layout.h"

#define PMM_PAGE_SIZE 4096
#define KERNEL_VIRT_BASE 0xC0000000

#define CURRENT_PD ((uint32_t*)0xFFFFF000)
#define PT_BASE ((uint32_t *)0xFFC00000)

extern uint8_t _kstart;
extern uint8_t _kend;

uint32_t *temp_window = NULL;

page_directory_t *vm_get_current_address_space(void)
{
	uint32_t value;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(value));
	return (page_directory_t *)(value & 0xFFFFF000);
}

static void invlpg(void *addr)
{
    __asm__ volatile ("invlpg (%0)" :: "r"(addr) : "memory");
}

void vm_invalidate_tlb(void)
{
	uint32_t cr3;

	__asm__ volatile (
		"mov %%cr3, %0"
		: "=r" (cr3)
	);

	__asm__ volatile (
		"mov %0, %%cr3"
		:
		: "r" (cr3)
		: "memory"
	);
}

static bool vm_temp_map_page(
	phys_addr_t phys,
	page_flags_t flags)
{
	assert((flags & ~0x0FFF) == 0);
	assert((phys & 0x0FFF) == 0);

	uint32_t pde_index = (uint32_t)temp_window >> 22;
	uint32_t pte_index = ((uint32_t)temp_window >> 12) & 0x3FF;

	if ((CURRENT_PD[pde_index] & VM_PRESENT) == 0) {
		phys_addr_t pt_phys = (phys_addr_t)pmm_alloc();
		assert((void *)pt_phys != NULL);
		CURRENT_PD[pde_index] = pt_phys | VM_PRESENT | VM_WRITE;
		
		memset((PT_BASE + pde_index * PMM_PAGE_SIZE), 0, PMM_PAGE_SIZE);
	}

	((uint32_t *)((uint8_t *)PT_BASE + pde_index * PMM_PAGE_SIZE))[pte_index] = phys | flags | VM_PRESENT;

	invlpg(temp_window);

	return true;
}

void vm_init(void)
{
	vml_runtime_region_t temp_window_region = vml_vmem_temp_window_region();
	temp_window = (uint32_t *)temp_window_region.virt_base;
	
	page_directory_t *as = vm_create_address_space();

	uint8_t *start = NULL;
	uint8_t *end = ALIGN_UP(ALIGN_UP(&_kend + 1, PMM_PAGE_SIZE) + pmm_get_bitmap_size(), PMM_PAGE_SIZE) - KERNEL_VIRT_BASE;

	// map the first 4 MB of physical memory to upper half
	for (uint32_t offset = 0; offset < (uint32_t)end - (uint32_t)start; offset += 4096) {
		vm_map_page(
			as,
			(virt_addr_t)((uint8_t *)KERNEL_VIRT_BASE + offset),
			(phys_addr_t)(start + offset),
			VM_PRESENT | VM_WRITE
		);
	}

	// map last record to the PD itself
	vm_temp_map_page((phys_addr_t)as, VM_WRITE);
	temp_window[1023] = (uint32_t)as | VM_WRITE | VM_PRESENT;

	vm_set_address_space(as);
}

void vm_set_address_space(page_directory_t *pd)
{
	uint32_t phys = ((uint32_t)pd);
	assert((phys & 0x0FFF) == 0);
	__asm__ volatile ("mov %0, %%cr3" :: "r"(phys) : "memory");
}

page_directory_t *vm_create_address_space(void)
{
	page_directory_t *as = pmm_alloc();
	assert(as != NULL);

	vm_temp_map_page((phys_addr_t)as, VM_WRITE);
	for (uint32_t i = 0; i < 1024; ++i) {
		temp_window[i] = 0;
	}

	return (page_directory_t *)as;
}

bool vm_map_page(
	page_directory_t *pd,
	virt_addr_t virt,
	phys_addr_t phys,
	page_flags_t flags)
{
	assert((flags & ~0x0FFF) == 0);
	assert((virt & 0x0FFF) == 0);
	assert((phys & 0x0FFF) == 0);

	uint32_t pde_index = virt >> 22;
	uint32_t pte_index = (virt >> 12) & 0x3FF;

	vm_temp_map_page((phys_addr_t)pd, VM_WRITE);

	if ((temp_window[pde_index] & VM_PRESENT)) {
		vm_temp_map_page((phys_addr_t)(temp_window[pde_index] & ~0x0FFF), VM_WRITE);
	} else {
		phys_addr_t pt_phys = (phys_addr_t)pmm_alloc();
		assert((void *)pt_phys != NULL);
		temp_window[pde_index] = pt_phys | VM_PRESENT | VM_WRITE;

		vm_temp_map_page((phys_addr_t)pt_phys, VM_WRITE);
		memset(temp_window, 0, PMM_PAGE_SIZE);
	}

	temp_window[pte_index] = phys | flags | VM_PRESENT;

	return true;
}

void vm_unmap_page(
	page_directory_t *pd,
	virt_addr_t virt)
{
	assert((virt & 0x0FFF) == 0);

	uint32_t pde_index = virt >> 22;
	uint32_t pte_index = (virt >> 12) & 0x3FF;

	vm_temp_map_page((phys_addr_t)pd, VM_WRITE);
	if ((temp_window[pde_index] & VM_PRESENT) == 0) return;
	uint32_t *pt = (uint32_t *)(temp_window[pde_index] & ~0x0FFF);

	vm_temp_map_page((phys_addr_t)pt, VM_WRITE);
	temp_window[pte_index] = 0;
}

bool vm_alloc_page(page_directory_t *pd,
	virt_addr_t virt,
	page_flags_t flags)
{
	void *page = pmm_alloc();
	assert(page != NULL);
	return vm_map_page(pd, (virt_addr_t)virt, (phys_addr_t)page, flags);
}
