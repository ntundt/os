#ifndef KERNEL_STDLIB_H
#define KERNEL_STDLIB_H

#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *s);
char *strstr(const char *s1, const char *s2);
int atoi(const char *s);
void itoa(int n, char *s, int radix);

#endif
