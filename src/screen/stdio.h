#ifndef STDIO_H
#define STDIO_H

#include <limits.h>

#define STDIO_BUFFER_POS_END INT_MAX
#define STDIO_BUFFER_POS_START INT_MIN

int printf(const char *fmt, ...);

int printc(const char *fmt, ...);

char* gets_s(char *buffer, int size);

int init_stdio(void);

#ifdef DEBUG
#define printfd(...) printf("DEBUG: " __VA_ARGS__)
#else
#define printfd(...)
#endif

#endif
