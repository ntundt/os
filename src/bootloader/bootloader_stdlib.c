#include "bootloader_stdlib.h"

void *memcpy(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;
	while (n--) {
		*d++ = *s++;
	}
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	char *p = s;
	while (n--) {
		*p++ = c;
	}
	return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	while (n--) {
		if (*p1 != *p2) {
			return *p1 - *p2;
		}
		p1++;
		p2++;
	}
	return 0;
}

size_t strlen(const char *s)
{
	size_t len = 0;
	while (*s++) {
		len++;
	}
	return len;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
	char *d = dest;
	while (*src) {
		*d++ = *src++;
	}
	*d = 0;
	return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
	char* d = dest;
	while (*src && n) {
		*d++ = *src++;
		n--;
	}
	*d = 0;
	return dest;
}

int atoi(const char *s)
{
	int n = 0;
	while (*s) {
		n = n * 10 + *s - '0';
		s++;
	}
	return n;
}

void itoa(int n, char *s, int radix)
{
	char *p = s;
	char *firstdig;
	char temp;
	unsigned digval;
	if (n < 0) {
		*p++ = '-';
		n = -n;
	}
	firstdig = p;
	do {
		digval = (unsigned)(n % radix);
		n /= radix;
		if (digval > 9) {
			*p++ = (char)(digval - 10 + 'a');
		} else {
			*p++ = (char)(digval + '0');
		}
	} while (n > 0);
	*p-- = '\0';
	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;
		--p;
		++firstdig;
	} while (firstdig < p);
}
