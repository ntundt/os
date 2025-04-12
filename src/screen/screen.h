#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define VIDEO_MEMORY ((char *)0xB8000)
#define VIDEO_MEMORY_SIZE 0xFA0
#define VIDEO_MEMORY_END (VIDEO_MEMORY + VIDEO_MEMORY_SIZE)

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define COLOR_BLACK ((uint8_t)0x0)
#define COLOR_BLUE ((uint8_t)0x1)
#define COLOR_GREEN ((uint8_t)0x2)
#define COLOR_CYAN ((uint8_t)0x3)
#define COLOR_RED ((uint8_t)0x4)
#define COLOR_MAGENTA ((uint8_t)0x5)
#define COLOR_BROWN ((uint8_t)0x6)
#define COLOR_LIGHT_GREY ((uint8_t)0x7)
#define COLOR_DARK_GREY ((uint8_t)0x8)
#define COLOR_LIGHT_BLUE ((uint8_t)0x9)
#define COLOR_LIGHT_GREEN ((uint8_t)0xA)
#define COLOR_LIGHT_CYAN ((uint8_t)0xB)
#define COLOR_LIGHT_RED ((uint8_t)0xC)
#define COLOR_LIGHT_MAGENTA ((uint8_t)0xD)
#define COLOR_YELLOW ((uint8_t)0xE)
#define COLOR_WHITE ((uint8_t)0xF)

#define BACKGROUD_BLACK ((uint8_t)(COLOR_BLACK << 4))
#define BACKGROUD_BLUE ((uint8_t)(COLOR_BLUE << 4))
#define BACKGROUD_GREEN ((uint8_t)(COLOR_GREEN << 4))
#define BACKGROUD_CYAN ((uint8_t)(COLOR_CYAN << 4))
#define BACKGROUD_RED ((uint8_t)(COLOR_RED << 4))
#define BACKGROUD_MAGENTA ((uint8_t)(COLOR_MAGENTA << 4))
#define BACKGROUD_BROWN ((uint8_t)(COLOR_BROWN << 4))
#define BACKGROUD_LIGHT_GREY ((uint8_t)(COLOR_LIGHT_GREY << 4))
#define BACKGROUD_DARK_GREY ((uint8_t)(COLOR_DARK_GREY << 4))
#define BACKGROUD_LIGHT_BLUE ((uint8_t)(COLOR_LIGHT_BLUE << 4))
#define BACKGROUD_LIGHT_GREEN ((uint8_t)(COLOR_LIGHT_GREEN << 4))
#define BACKGROUD_LIGHT_CYAN ((uint8_t)(COLOR_LIGHT_CYAN << 4))
#define BACKGROUD_LIGHT_RED ((uint8_t)(COLOR_LIGHT_RED << 4))
#define BACKGROUD_LIGHT_MAGENTA ((uint8_t)(COLOR_LIGHT_MAGENTA << 4))
#define BACKGROUD_YELLOW ((uint8_t)(COLOR_YELLOW << 4))
#define BACKGROUD_WHITE ((uint8_t)(COLOR_WHITE << 4))

void screen_put_char(int row, int column, char c, uint8_t color);

void screen_get_char(int row, int column, char *c, uint8_t *color);

void screen_set_cursor(int row, int column);

void screen_get_cursor(int *row, int *column);

void screen_toggle_cursor(int visible);

void screen_clear();

#endif
