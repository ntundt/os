#include "screen/screen.h"
#include "screen/stdio.h"
#include "screen/panic.h"
#include "interrupts.h"
#include "kernel_stdlib.h"
#include "fs/fat16.h"
#include "fs/fat16drv.h"
#include "fs/floppy.h"
#include "ps2/keyboard.h"
#include "cpuio/cpuio.h"
#include "multiboot.h"
#include "gdt/gdt.h"
#include "pmm.h"
#include "vmem.h"

#define IOBUF_SIZE 512

static void reboot(void)
{
	outb(0x64, 0xFE);
	printf("Rebooting...\n");
	while (1) { }
}

#define KERNEL_STACK_SIZE 16384

static uint8_t kernel_stack[KERNEL_STACK_SIZE];

void kernel_main(uint32_t multiboot_magic, void *multiboot_info)
{
	if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		return;

	parse_multiboot(multiboot_info);

	gdt_init((void *)kernel_stack);
	pmm_init();
	vm_init();

	stdio_init();
	screen_clear();
	screen_set_cursor(0, 0);

	printf("My OS v0.0.3\n"
		"Detected %d bytes of RAM\n\n"
		"Welcome to my OS!\n\n"
		"1 - Reboot\n"
		"Please choose wisely: ", get_highest_addr());

	if (ps2_controller_present())
		panic("PS/2 controller not present. There's no way to input anything.\n"
		"Other input devices are not supported yet.\n");

	trap_idt_setup();
	ps2_init();
	irq_enable();

	char answer[16];
	while (1) {
		gets_s(answer, sizeof(answer));
		if (strcmp(answer, "1") == 0) {
			reboot();
		} else {
			memset(answer, 0, sizeof(answer));
			printf("Please choose wisely: ");
		}
	}
}
