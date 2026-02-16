#include "stdio.h"
#include "screen.h"
#include "kernel/kernel_stdlib.h"
#include "ps2/keyboard.h"

#include <limits.h>
#include <stdarg.h>

static void screen_scroll(void)
{
	char c;
	uint8_t attr;
	for (int row = 1; row < 25; row++) {
		for (int col = 0; col < 80; col++) {
			screen_get_char(row, col, &c, &attr);
			screen_put_char(row - 1, col, c, attr);
		}
	}
	for (int col = 0; col < 80; col++) {
		screen_put_char(24, col, ' ', 0x07);
	}
}

static void put_char(char c)
{
	int row, col;
	screen_get_cursor(&row, &col);
	if (c == '\n') {
		row++;
		col = 0;
	} else {
		screen_put_char(row, col, c, 0x07);
		col++;
	}
	if (col >= 80) {
		col = 0;
		row++;
	}
	if (row >= 25) {
		screen_scroll();
		row = 24;
	}
	screen_set_cursor(row, col);
}

static void put_string(const char *s)
{
	while (*s) {
		put_char(*s++);
	}
}

static void put_int(int n)
{
	char buf[16];
	itoa(n, buf, 10);
	put_string(buf);
}

static void put_hex(int n)
{
	char buf[16];
	itoa(n, buf, 16);
	put_string(buf);
}

int printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
			case 's':
				put_string(va_arg(ap, char *));
				break;
			case 'd':
				put_int(va_arg(ap, int));
				break;
			case 'x':
				put_hex(va_arg(ap, int));
				break;
			default:
				fmt--;
				put_char(*fmt);
				break;
			}
		} else {
			put_char(*fmt);
		}
		
		fmt++;
	}
	va_end(ap);

	return 0;
}

static int vprintf(const char *fmt, char* ap)
{
	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
			case 's':
				put_string(*(char**)ap);
				ap += sizeof(char*);
				break;
			case 'd':
				put_int(*(int*)ap);
				ap += sizeof(int);
				break;
			case 'x':
				put_hex(*(int*)ap);
				ap += sizeof(int);
				break;
			default:
				fmt--;
				put_char(*fmt);
				break;
			}
		} else {
			put_char(*fmt);
		}
		
		fmt++;
	}

	return 0;
}

// prints s to the screen at (0, 0) with printf, then returns the cursor to its
// original position
int printc(const char *s, ...)
{
	va_list ap;
	int row, col;

	screen_get_cursor(&row, &col);
	screen_set_cursor(0, 0);

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);

	screen_set_cursor(row, col);

	return 0;
}

static char *gets_buffer = NULL;
static int gets_buffer_size = 0;
static int input_ended = 0;
static int resulting_string_size = 0;
static int cursor_pos = 0;

static int buffer_initial_row = 0;
static int buffer_intital_col = 0;

char* gets_s(char *buf, int size)
{
	screen_get_cursor(&buffer_initial_row, &buffer_intital_col);

	gets_buffer = buf;
	gets_buffer_size = size;
	input_ended = 0;
	resulting_string_size = 0;
	cursor_pos = 0;

	while (!input_ended) {
		// do nothing
	}

	gets_buffer = NULL;
	gets_buffer_size = 0;
	input_ended = 0;
	resulting_string_size = 0;
	cursor_pos = 0;

	put_char('\n');

	return buf;
}

static void redraw_gets_buffer()
{
	screen_set_cursor(buffer_initial_row, buffer_intital_col);

	for (int i = 0; i < resulting_string_size; i++)
	{
		put_char(gets_buffer[i]);
	}
	put_char(' ');

	screen_set_cursor(buffer_initial_row, buffer_intital_col + cursor_pos);
}

static void buffer_insert_char(char c)
{
	if (resulting_string_size + 1 > gets_buffer_size) return;
	if (resulting_string_size != cursor_pos) {
		for (int i = resulting_string_size; i >= cursor_pos; i--) {
			gets_buffer[i] = gets_buffer[i - 1];
		}
	}
	gets_buffer[cursor_pos++] = c;
	resulting_string_size++;
}

static void buffer_remove_char(int forward)
{
	if (resulting_string_size == 0) return;

	if (forward && cursor_pos == resulting_string_size) return;
	if (!forward && cursor_pos == 0) return;

	int i = forward ? cursor_pos : cursor_pos - 1;
	for (; i < resulting_string_size + 1; i++) {
		gets_buffer[i] = gets_buffer[i + 1];
	}
	resulting_string_size--;
	if (!forward) cursor_pos--;
}

static void buffer_move_cursor(int offset)
{
	if (offset == STDIO_BUFFER_POS_END) {
		cursor_pos = resulting_string_size;
		return;
	} else if (offset == STDIO_BUFFER_POS_START) {
		cursor_pos = 0;
		return;
	}

	cursor_pos += offset;
	if (cursor_pos > resulting_string_size) {
		cursor_pos = resulting_string_size;
	} else if (cursor_pos < 0) {
		cursor_pos = 0;
	}
}

static void on_key(uint16_t scancode, uint8_t pressed)
{
	if (gets_buffer == NULL) return;

	static int ctrl_pressed = 0;
	static int caps_lock_enabled = 0;
	static int shift_pressed = 0;

	(void) ctrl_pressed;
	
	switch (scancode) {
	case PS2_KEY_ENTER:
		if (pressed) {
			input_ended = 1;
			gets_buffer[resulting_string_size] = '\0';
		}
		goto on_key_end;
	case PS2_KEY_BACKSPACE:
		if (pressed) buffer_remove_char(0);
		goto on_key_end;
	case PS2_KEY_DELETE:
		if (pressed) buffer_remove_char(1);
		goto on_key_end;
	case PS2_KEY_END:
		if (pressed) buffer_move_cursor(STDIO_BUFFER_POS_END);
		goto on_key_end;
	case PS2_KEY_HOME:
		if (pressed) buffer_move_cursor(STDIO_BUFFER_POS_START);
		goto on_key_end;
	case PS2_KEY_LEFT:
		if (pressed) buffer_move_cursor(-1);
		goto on_key_end;
	case PS2_KEY_RIGHT:
		if (pressed) buffer_move_cursor(1);
		goto on_key_end;
	case PS2_KEY_LEFT_SHIFT:
	case PS2_KEY_RIGHT_SHIFT:
		shift_pressed = pressed;
		goto on_key_end;
	case PS2_KEY_LEFT_CTRL:
	case PS2_KEY_RIGHT_CTRL:
		ctrl_pressed = pressed;
		goto on_key_end;
	case PS2_KEY_CAPS_LOCK:
		if (pressed) caps_lock_enabled = !caps_lock_enabled;
		goto on_key_end;
	}

	if (!pressed) goto on_key_end;

	char c;
	int uppercase = caps_lock_enabled ? (shift_pressed ? 0 : 1) : (shift_pressed ? 1 : 0); 
	if (ps2_kb_scancode_to_ascii(scancode, &c)) {
		buffer_insert_char(uppercase ? (uint8_t)c ^ 0x20 : c);
	}

on_key_end:
	redraw_gets_buffer();
}

int stdio_init(void)
{
	return ps2_kb_set_callback(on_key);
}
