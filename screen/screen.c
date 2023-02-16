#include "screen.h"
#include "../cpuio/cpuio.h"

void screen_put_char(int row, int column, char c, uint8_t color)
{
	VIDEO_MEMORY[(row * SCREEN_WIDTH + column) * 2] = c;
	VIDEO_MEMORY[(row * SCREEN_WIDTH + column) * 2 + 1] = color;
}

void screen_get_char(int row, int column, char *c, uint8_t *color)
{
	*c = VIDEO_MEMORY[(row * SCREEN_WIDTH + column) * 2];
	*color = VIDEO_MEMORY[(row * SCREEN_WIDTH + column) * 2 + 1];
}

void screen_set_cursor(int row, int column)
{
	uint16_t position = (row * SCREEN_WIDTH) + column;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(position & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void screen_get_cursor(int *row, int *column)
{
	uint16_t position;

	outb(0x3D4, 0x0F);
	position = inb(0x3D5);
	outb(0x3D4, 0x0E);
	position |= ((uint16_t)inb(0x3D5)) << 8;

	*row = position / SCREEN_WIDTH;
	*column = position % SCREEN_WIDTH;
}

void screen_toggle_cursor(int visible)
{
	if (visible) {
		outb(0x3D4, 0x0A);
		outb(0x3D5, (inb(0x3D5) & 0xC0) | 0x0D);
		outb(0x3D4, 0x0B);
		outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x0E);
	} else {
		outb(0x3D4, 0x0A);
		outb(0x3D5, 0x20);
	}
}

void screen_clear()
{
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		VIDEO_MEMORY[i * 2] = ' ';
		VIDEO_MEMORY[i * 2 + 1] = 0x0f;
	}
}
