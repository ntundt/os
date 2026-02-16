#ifndef KEYBOARD_H
#define KEYBOARD_H

#define PS2_LED_SCROLL_LOCK 0x01
#define PS2_LED_NUM_LOCK 0x02
#define PS2_LED_CAPS_LOCK 0x04

#define PS2_KEY_BACKSPACE 0x66
#define PS2_KEY_DELETE 0xE071
#define PS2_KEY_HOME 0xE06C
#define PS2_KEY_END 0xE069
#define PS2_KEY_LEFT_SHIFT 0x12
#define PS2_KEY_RIGHT_SHIFT 0x59
#define PS2_KEY_LEFT_CTRL 0x14
#define PS2_KEY_RIGHT_CTRL 0xE014
#define PS2_KEY_LEFT_ALT 0x11
#define PS2_KEY_RIGHT_ALT 0xE011
#define PS2_KEY_CAPS_LOCK 0x58
#define PS2_KEY_ENTER 0x5A
#define PS2_KEY_LEFT 0xE06B
#define PS2_KEY_RIGHT 0xE074

#include <stdint.h>

int ps2_controller_present(void);

void ps2_init(void);

int ps2_kb_set_callback(void (*callback)(uint16_t, uint8_t));

int ps2_kb_unset_callback(void);

int ps2_kb_scancode_to_ascii(uint16_t scancode, char *ascii);

#endif
