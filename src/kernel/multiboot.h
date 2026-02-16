#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289
#define MAX_MEMORY_REGIONS 1024

struct multiboot_tag {
	uint32_t type;
	uint32_t size;
};

struct multiboot_tag_mmap {
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
};

struct multiboot_mmap_entry {
	uint64_t addr;
	uint64_t len;
	uint32_t type;
	uint32_t zero;
};

/* ACPI RSDP tag */
struct multiboot_tag_rsdp {
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[0]; /* pointer to RSDP table (physical address) */
};

/* Store usable memory regions */
struct multiboot_memory_region {
	uint64_t base;
	uint64_t length;
};

void parse_multiboot(void* mb_info);

size_t get_usable_memory_count(void);

struct multiboot_memory_region *get_usable_memory(void);

void* get_rsdp_old(void);

void* get_rsdp_new(void);

void *get_highest_addr(void);

#endif
