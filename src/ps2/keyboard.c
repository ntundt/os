#include "keyboard.h"
#include "cpuio/cpuio.h"
#include "bootloader/bootloader_stdlib.h"
#include "interrupts.h"
#include "screen/stdio.h"

int ps2_controller_present(void)
{
	uint8_t *acpi_table = (uint8_t *)0x000E0000;
	uint8_t acpi_table_target[] = "RSD PTR ";

	if (memcmp(acpi_table, acpi_table_target, 8) != 0) {
		return 0;
	}

	if (acpi_table[109] & 0x02) {
		return 1;
	}

	return 0;
}

void ps2_kb_init(void)
{
	// Disable keyboard
	outb(0x64, 0xAD);
	outb(0x64, 0xA7);

	// Flush the output buffer
	while (inb(0x64) & 0x01) {
		inb(0x60);
	}

	outb(0x64, 0x20);
	uint8_t config = inb(0x60);
	config &= 0x3F;
	outb(0x64, 0x60);
	outb(0x60, config);

	outb(0x64, 0xAA);
	if (inb(0x60) != 0x55) {
		return;
	}

	if (config & 0x20) {
		outb(0x64, 0xA8);
		outb(0x64, 0x20);
		config = inb(0x60);
		config &= 0x3F;
		outb(0x64, 0x60);
		outb(0x60, config);
		if (config & 0x20) {
			return;
		}
		outb(0x64, 0xA7);
	} else {
		outb(0x64, 0xA7);
	}

	outb(0x64, 0xAB);
	if (inb(0x60) != 0x00) {
		return;
	}

	outb(0x64, 0xAE);
	outb(0x64, 0xA8);
	config |= 0x03;
	outb(0x64, 0x60);
	outb(0x60, config);

	outb(0x60, 0xFF);
	outb(0x60, 0xFF);
}

void ps2_kb_set_leds(uint8_t leds)
{
	outb(0x60, 0xED);
	outb(0x60, leds);
}

const char PS2_to_ASCII[] = {
     /* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\t', '`',   0, // 0x00
	0,   0,   0,   0,   0, 'q', '1',   0,   0,   0, 'z', 's', 'a', 'w', '2',   0, // 0x10
	0, 'c', 'x', 'd', 'e', '4', '3',   0,   0, ' ', 'v', 'f', 't', 'r', '5',   0, // 0x20
	0, 'n', 'b', 'h', 'g', 'y', '6',   0,   0,   0, 'm', 'j', 'u', '7', '8',   0, // 0x30
	0, ',', 'k', 'i', 'o', '0', '9',   0,   0, '.', '/', 'l', ';', 'p', '-',   0, // 0x40
	0,   0,'\'',   0, '[', '=',   0,   0,   0,   0,   0, ']',   0,'\\',   0,   0, // 0x50
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0x60
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0x70
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0x80
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0x90
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0xA0
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0xB0
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0xC0
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0xD0
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0xE0
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0  // 0xF0
};

static void (*kb_callback)(uint16_t, uint8_t) = NULL;

int ps2_kb_set_callback(void (*callback)(uint16_t, uint8_t))
{
	if (callback == NULL)
		return -1;

	kb_callback = callback;
	return 0;
}

int ps2_kb_unset_callback(void)
{
	if (kb_callback == NULL)
		return -1;

	kb_callback = NULL;
	return 0;
}


int ps2_kb_scancode_to_ascii(uint16_t scancode, char *ascii)
{
	static int next_release = 0;
	static int extended = 0;

	if (scancode == 0xE0) {
		extended = 1;
		return 0;
	}

	if (scancode == 0xF0) {
		next_release = 1;
		return 0;
	}

	if (next_release) {
		next_release = 0;
		return 0;
	}

	if (extended) {
		extended = 0;
		return 0;
	}

	if (scancode & 0x80) {
		return 0;
	}

	*ascii = PS2_to_ASCII[scancode];
	return 1;
}

void keyboard_handler(uint16_t scancode, uint8_t pressed)
{
	if (kb_callback != NULL)
		kb_callback(scancode, pressed);
}

__attribute__((interrupt)) void kb_default_irq_handler(void *stack)
{
	(void) stack;

	uint8_t scancode = inb(0x60);

	static int next_release = 0;
	static int extended = 0;
	int pressed = 1;

	if (scancode == 0xE0) {
		extended = 1;
		goto kb_int_handler_end;
	}

	if (scancode == 0xF0) {
		next_release = 1;
		goto kb_int_handler_end;
	}

	if (next_release) {
		next_release = 0;
		pressed = 0;
	} else {
		pressed = 1;
	}
	keyboard_handler((extended * (0xE0 << 8)) | scancode, pressed);

	extended = 0;

kb_int_handler_end:
	outb(0x20, 0x20);
}

void ps2_kb_set_default_irq_handler()
{
	trap_set_gate(33, kb_default_irq_handler);
}
