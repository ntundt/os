#include "panic.h"
#include "stdio.h"

__attribute__((noreturn)) void panic(const char *message)
{
	printf("PANIC: %s", message);
	__asm__ __volatile__ ("cli");
	__asm__ __volatile__ ("hlt");

	while (1) { __asm__ __volatile__ ("nop"); } // to suppress -Winvalid-noreturn
}
