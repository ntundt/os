#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"

#define KERNEL_VIRTUAL_BASE ((uint8_t *)0xC0000000)

static struct multiboot_memory_region usable_regions[MAX_MEMORY_REGIONS];
static size_t usable_count = 0;

static void *highest_addr = NULL;

/* ACPI RSDP pointers */
static void* rsdp_old = NULL;
static void* rsdp_new = NULL;

void parse_multiboot(void* mb_info)
{
	uint8_t* ptr = (uint8_t*)mb_info;
	ptr += 8; /* skip total_size + reserved */

	while (1) {
		struct multiboot_tag* tag = (struct multiboot_tag*)ptr;
		if (tag->type == 0) /* end tag */
			break;

		switch (tag->type) {
		case 6: { /* memory map */
			struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap*)tag;
			uint8_t* entry_ptr = (uint8_t*)mmap + sizeof(*mmap);
			while (entry_ptr < (uint8_t*)tag + mmap->size) {
				struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)entry_ptr;
				if (entry->type == 1 && usable_count < MAX_MEMORY_REGIONS) {
					usable_regions[usable_count].base = entry->addr;
					usable_regions[usable_count].length = entry->len;
					usable_count++;

					if (entry->addr + entry->len > (uint32_t)highest_addr)
						highest_addr = (void *)(uint32_t)(entry->addr + entry->len);
				}
				entry_ptr += mmap->entry_size;
			}
			break;
		}
		case 14: /* ACPI old RSDP */
			rsdp_old = (void*)((struct multiboot_tag_rsdp*)tag)->rsdp;
			break;
		case 15: /* ACPI new RSDP */
			rsdp_new = (void*)((struct multiboot_tag_rsdp*)tag)->rsdp;
			break;
		default:
			/* ignore other tags */
			break;
		}

		/* Move to next tag (8-byte aligned) */
		ptr += (tag->size + 7) & ~7;
	}
}

size_t get_usable_memory_count(void) {
	return usable_count;
}

struct multiboot_memory_region *get_usable_memory(void) {
	return usable_regions;
}

void *get_rsdp_old(void) {
	return (void *)(rsdp_old + (uint32_t)KERNEL_VIRTUAL_BASE);
}

void *get_rsdp_new(void) {
	return (void *)(rsdp_new + (uint32_t)KERNEL_VIRTUAL_BASE);
}

void *get_highest_addr(void) {
	return highest_addr;
}
