/**
 * Loads the kernel into memory and jumps to it.
 */

#include "screen/screen.h"
#include "screen/stdio.h"
#include "screen/panic.h"
#include "interrupts.h"
#include "bootloader_stdlib.h"
#include "fs/floppy.h"
#include "ps2/keyboard.h"
#include "cpuio/cpuio.h"

#define KERNEL_LOAD_ADDRESS (void*)0x100000
#define KERNEL_ENTRY_ADDRESS 0x100000

void reboot(void)
{
	outb(0x64, 0xFE);
	printf("Rebooting...\n");
	while (1) { }
}

void bootloader_main(void)
{
	screen_clear();

	screen_set_cursor(0, 0);

	setup_idt();

	printf("My OS bootloader v0.0.1\n\n");

	if (ps2_controller_present() != 0) {
		panic("PS/2 controller not present. There's no way to input anything.\n"
			"Other input devices are not supported yet.\n");
	}

	ps2_kb_init();

	printf("You're about to boot into my OS.\n\n");

	printf("1 - Boot into my OS\n");
	printf("2 - Reboot\n");

	printf("Please choose wisely: ");

	ps2_kb_set_default_irq_handler();

	enable_interrupts();

	init_stdio();

	char answer[16];
	while (1) {
		gets_s(answer, sizeof(answer));
		if (strcmp(answer, "1") == 0) {
			goto load_kernel;
		} else if (strcmp(answer, "2") == 0) {
			reboot();
		} else {
			memset(answer, 0, sizeof(answer));
			printf("Please choose wisely: ");
		}
	}

load_kernel:
	printf("Loading kernel...\n");

	panic("no bootable kernel image found");

	void (*kernel_entry)() = (void (*)())KERNEL_ENTRY_ADDRESS;
	kernel_entry();
}
