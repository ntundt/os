#include <stdint.h>
#include "gdt.h"

struct gdt_entry gdt[4];
struct gdt_ptr gp;
struct tss_entry tss;

void gdt_set_gate(
	int num,
	uint32_t base,
	uint32_t limit,
	uint8_t access,
	uint8_t gran
) {
	gdt[num].base_low    = (base & 0xFFFF);
	gdt[num].base_mid    = (base >> 16) & 0xFF;
	gdt[num].base_high   = (base >> 24) & 0xFF;

	gdt[num].limit_low   = (limit & 0xFFFF);
	gdt[num].granularity = (limit >> 16) & 0x0F;

	gdt[num].granularity |= gran & 0xF0;
	gdt[num].access      = access;
}

void gdt_init(void *stack_top)
{
	gp.limit = sizeof(gdt) - 1;
	gp.base  = (uint32_t)&gdt;

	// Null
	gdt_set_gate(0, 0, 0, 0, 0);

	// Kernel code
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	// Kernel data
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	// Setup TSS
	tss.esp0 = (uint32_t)stack_top;
	tss.ss0  = 0x10;

	uint32_t base  = (uint32_t)&tss;
	uint32_t limit = sizeof(tss);

	gdt_set_gate(3, base, limit, 0x89, 0x40);

	__asm__ __volatile__("lgdt %0" : : "m"(gp));

	// Reload segments
	__asm__ __volatile__(
		"mov $0x10, %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"mov %%ax, %%ss\n"
		"ljmp $0x08, $.reload\n"
		".reload:\n"
		:
		:
		: "ax"
	);

	// Load TSS
	__asm__ __volatile__(
		"mov $0x18, %%ax\n"
		"ltr %%ax\n"
		:
		:
		: "ax"
	);
}
