#ifndef PRINTF_H
#define PRINTF_H

#include <limits.h>

#define STDIO_BUFFER_POS_END INT_MAX
#define STDIO_BUFFER_POS_START INT_MIN

int printf(const char *fmt, ...);

int printc(const char *fmt, ...);

char* gets_s(char *buffer, int size);

int init_stdio(void);

#endif
