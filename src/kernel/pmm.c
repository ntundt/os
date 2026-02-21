#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "pmm.h"
#include "multiboot.h"
#include "kernel_stdlib.h"
#include "screen/panic.h"
#include "memory_helpers.h"

extern uint8_t _kstart;
extern uint8_t _kend;
#define KERNEL_VIRT_BASE ((uint8_t *)0xC0000000)

static void* pmm_bitmap_start = NULL;
static uint32_t pmm_bitmap_length = 0;

static void pmm_set_usable(void *page)
{
	uint32_t page_index = (uint32_t)page / PMM_PAGE_SIZE;
	uint8_t *byte = (uint8_t *)pmm_bitmap_start + page_index / 8;
	uint8_t bit = page_index % 8;

	*byte &= ~(1 << bit);
}

static void pmm_set_unusable(void *page)
{
	uint32_t page_index = (uint32_t)page / PMM_PAGE_SIZE;
	uint8_t *byte = (uint8_t *)pmm_bitmap_start + page_index / 8;
	uint8_t bit = page_index % 8;
	
	*byte |= (1 << bit);
}

static bool pmm_is_usable(void *page)
{
	uint32_t page_index = (uint32_t)page / PMM_PAGE_SIZE;
	uint8_t *byte = (uint8_t *)pmm_bitmap_start + page_index / 8;
	uint8_t bit = page_index % 8;

	return (*byte & (1 << bit)) == 0;
}

void pmm_init(void)
{
	void *highest_addr = get_highest_addr();
	uint32_t total_pages = (uint32_t)highest_addr / PMM_PAGE_SIZE;
	uint32_t bitmap_size_bytes = (total_pages + 7) / 8;
	pmm_bitmap_start = ALIGN_UP(&_kend + 1, PMM_PAGE_SIZE);
	pmm_bitmap_length = bitmap_size_bytes;
	// set everything as unusable
	memset(pmm_bitmap_start, 0xFF, pmm_bitmap_length);

	// set memory regions provided by multiboot as usable
	struct multiboot_memory_region *usable_memory = get_usable_memory();
	for (uint32_t i = 0; i < get_usable_memory_count(); ++i) {
		uint32_t start = usable_memory[i].base;
		uint32_t end = usable_memory[i].base + usable_memory[i].length;

		if (end <= 0x100000) continue;
		if (start < 0x100000) start = 0x100000;

		void *page = ALIGN_UP((void*)start, PMM_PAGE_SIZE);
		void *region_end = ALIGN_DOWN((void*)end, PMM_PAGE_SIZE);
		while (page < region_end) {
			pmm_set_usable(page);
			page = (void*)((size_t)page + PMM_PAGE_SIZE);
		}
	}

	// set memory regions occupied by the kernel and PMM bitmap as unusable
	void *page = ALIGN_DOWN(&_kstart - KERNEL_VIRT_BASE, PMM_PAGE_SIZE);
	while (page < ALIGN_UP((uint8_t *)pmm_bitmap_start + pmm_bitmap_length - KERNEL_VIRT_BASE, PMM_PAGE_SIZE)) {
		pmm_set_unusable(page);
		page += PMM_PAGE_SIZE;
	}
}

uint32_t pmm_get_bitmap_size(void)
{
	return pmm_bitmap_length;
}

static void *pmm_get_highest_page(void)
{
	return (void *)(pmm_bitmap_length * 8 * PMM_PAGE_SIZE);
}

void *pmm_alloc(void)
{
	void *page = (void*)PMM_PAGE_SIZE;
	while ((uint32_t)page <= (uint32_t)pmm_get_highest_page()) {
		if (pmm_is_usable(page)) {
			pmm_set_unusable(page);
			return page;
		}
		page = (void*)((uint32_t)page + PMM_PAGE_SIZE);
	}
	return NULL;
}

void pmm_free(void *page)
{
	assert(!pmm_is_usable(page));
	pmm_set_usable(page);
}
