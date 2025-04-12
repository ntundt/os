#include "panic.h"
#include "stdio.h"

void panic(const char *message)
{
	printf("PANIC: %s", message);
	__asm__ __volatile__ ("cli");
	__asm__ __volatile__ ("hlt");
}
