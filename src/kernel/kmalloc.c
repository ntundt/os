#include <stdint.h>
#include <stdbool.h>
#include "kmalloc.h"
#include "vmem.h"
#include "vmem_layout.h"
#include "pmm.h"
#include "screen/panic.h"

struct buddy_block {
	bool is_free;
	struct buddy_block *next;
};

static struct buddy_block *head;
static struct buddy_block *tail;

static void buddy_init(void)
{
	page_directory_t *pd = vm_get_current_address_space();

	vml_runtime_region_t kheap = vml_kernel_heap_region();

	for (size_t page = kheap.virt_base; page < kheap.virt_base + kheap.size; page += PMM_PAGE_SIZE) {
		vm_alloc_page(pd, page, VM_WRITE | VM_PRESENT);
	}

	vm_invalidate_tlb();

	*(struct buddy_block *)kheap.virt_base = (struct buddy_block){ .is_free = true, .next = NULL };
	head = tail = (struct buddy_block *)kheap.virt_base;
}

void kmalloc_init(void)
{
	buddy_init();
}

static size_t buddy_block_size(struct buddy_block *block)
{
	vml_runtime_region_t heap_region = vml_kernel_heap_region();
	return (size_t)((uintptr_t)(block->next ? block->next : (struct buddy_block *)(heap_region.virt_base + heap_region.size)) - (uintptr_t)block);
}

static struct buddy_block *find_best_fit_free_block(size_t min_size)
{
	struct buddy_block *best_block = NULL;
	struct buddy_block *current_block = head;
	do {
		bool current_block_is_better =
			(best_block == NULL
				|| ((buddy_block_size(current_block) - sizeof(struct buddy_block) >= min_size)
					&& buddy_block_size(best_block) > buddy_block_size(current_block))
			);
		if (current_block->is_free && current_block_is_better) {
			best_block = current_block;
		}
	} while ((current_block = current_block->next));
	return best_block;
}

static void buddy_split_block(struct buddy_block *block)
{
	struct buddy_block *half = (struct buddy_block *)((uintptr_t)block + buddy_block_size(block) / 2);
	half->is_free = true;
	half->next = block->next;
	block->next = half;
}

static void buddy_split_block_until_best_fit(struct buddy_block *block, size_t target_size)
{
	while (buddy_block_size(block) / 2 - sizeof(struct buddy_block) > target_size) {
		buddy_split_block(block);
	}
}

void *kmalloc(size_t size)
{
	struct buddy_block *best_block = find_best_fit_free_block(size);
	assert(best_block != NULL);
	best_block->is_free = false;
	buddy_split_block_until_best_fit(best_block, size);
	return (void *)((uintptr_t)best_block + sizeof(struct buddy_block));
}
