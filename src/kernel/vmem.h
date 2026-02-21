#ifndef VMEM_H
#define VMEM_H

#include <stdint.h>

typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;
typedef uint32_t page_flags_t;

typedef struct page_directory page_directory_t;

#define VM_PRESENT   (1u << 0)
#define VM_WRITE     (1u << 1)
#define VM_USER      (1u << 2)
#define VM_GLOBAL    (1u << 3)
#define VM_NOCACHE   (1u << 4)

void vm_init(void);
/*
	- Creates kernel page directory
	- Maps kernel higher half
	- Identity maps early region
	- Enables paging
*/

void vm_invalidate_tlb(void);

void vm_set_address_space(page_directory_t *pd);
page_directory_t *vm_get_current_address_space(void);

page_directory_t *vm_create_address_space(void);
void vm_destroy_address_space(page_directory_t *pd);

void vm_switch_address_space(page_directory_t *pd);

bool vm_map_page(
	page_directory_t *pd,
	virt_addr_t virt,
	phys_addr_t phys,
	page_flags_t flags
);

void vm_unmap_page(
	page_directory_t *pd,
	virt_addr_t virt
);

phys_addr_t vm_resolve(
	page_directory_t *pd,
	virt_addr_t virt
);

bool vm_alloc_page(
	page_directory_t *pd,
	virt_addr_t virt,
	page_flags_t flags
);

void vm_free_page(
	page_directory_t *pd,
	virt_addr_t virt
);

#endif
