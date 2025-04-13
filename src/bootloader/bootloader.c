/**
 * Loads the kernel into memory and jumps to it.
 */

#include "screen/screen.h"
#include "screen/stdio.h"
#include "screen/panic.h"
#include "interrupts.h"
#include "bootloader_stdlib.h"
#include "fs/fat16.h"
#include "fs/fat16drv.h"
#include "fs/floppy.h"
#include "ps2/keyboard.h"
#include "cpuio/cpuio.h"

#define KERNEL_LOAD_ADDRESS (void*)0x100000
#define KERNEL_ENTRY_ADDRESS 0x100000

#define IOBUF_SIZE 512

static void reboot(void)
{
	outb(0x64, 0xFE);
	printf("Rebooting...\n");
	while (1) { }
}

static uint8_t iobuf[IOBUF_SIZE];

void bootloader_main(void)
{
	screen_clear();

	screen_set_cursor(0, 0);

	trap_idt_setup();

	printf("My OS bootloader v0.0.1\n\n");

	if (ps2_controller_present() != 0) {
		panic("PS/2 controller not present. There's no way to input anything.\n"
			"Other input devices are not supported yet.\n");
	}

	ps2_kb_init();

	printf("You're about to boot into my OS.\n\n");

	printf("1 - Boot into my OS\n");
	printf("2 - Boot safely\n");
	printf("3 - Reboot\n");

	printf("Please choose wisely: ");

	ps2_kb_set_default_irq_handler();

	irq_enable();

	init_stdio();

	char answer[16];
	while (1) {
		gets_s(answer, sizeof(answer));
		if (strcmp(answer, "1") == 0) {
			goto load_kernel;
		} else if (strcmp(answer, "2") == 0) {
			goto load_kernel;
		} else if (strcmp(answer, "3") == 0) {
			reboot();
		} else {
			memset(answer, 0, sizeof(answer));
			printf("Please choose wisely: ");
		}
	}

load_kernel:
	printf("Loading the kernel...\n");

	fdc_init();

	blkdevdescr_t x = fat16_blkdev(0, 0);
	struct fat16_direntry dir_buffer[16];
	bool finished;
	size_t capacity = sizeof(dir_buffer) / sizeof(struct fat16_direntry);
	fat16_lsdir(&x, "/BOOT", 0, dir_buffer, &capacity, &finished);

	bool kernel_found = false;
	for (int i = 0; i < capacity; i++) {
		printf("%s %d\n", dir_buffer[i].name, dir_buffer[i].file_size_bytes);
		if (memcmp(dir_buffer[i].name, "VMLINUZ\x20\x20\x20\x20", 11) == 0) {
			kernel_found = true;
			break;
		}
	}

	if (!kernel_found) {
		panic("no kernel image found");
	}

	panic("kernel found but elfloading is not implmented yet");

	void (*kernel_entry)() = (void (*)())KERNEL_ENTRY_ADDRESS;
	kernel_entry();
}
