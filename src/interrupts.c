#include "interrupts.h"
#include "screen/stdio.h"
#include "bootloader/bootloader_stdlib.h"
#include "cpuio/cpuio.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define EXCEPTION_HANDLER_DEFINITION(number) \
	__attribute__((interrupt)) void exception_handler_##number(void *stack) \
	{ \
		printf("\n\nException " STR(number) " occured somehow."); \
		__asm__ __volatile__("hlt"); \
	}
#define EXCEPTION_HANDLER(number) exception_handler_##number

// Doesn't do anything, just a stub, program flow will continue after the interrupt
#define IRQ_HANDLER_STUB(number) \
	__attribute__((interrupt)) void irq_handler_##number(void *stack) \
	{ \
		outb(0x20, 0x20); \
	}
#define IRQ_HANDLER(number) irq_handler_##number

static struct interrupt_descriptor idt[256];
static struct idtr idtr = { 0, 0 };

EXCEPTION_HANDLER_DEFINITION(0)
EXCEPTION_HANDLER_DEFINITION(1)
EXCEPTION_HANDLER_DEFINITION(2)
EXCEPTION_HANDLER_DEFINITION(3)
EXCEPTION_HANDLER_DEFINITION(4)
EXCEPTION_HANDLER_DEFINITION(5)
EXCEPTION_HANDLER_DEFINITION(6)
EXCEPTION_HANDLER_DEFINITION(7)
EXCEPTION_HANDLER_DEFINITION(8)
EXCEPTION_HANDLER_DEFINITION(9)
EXCEPTION_HANDLER_DEFINITION(10)
EXCEPTION_HANDLER_DEFINITION(11)
EXCEPTION_HANDLER_DEFINITION(12)
EXCEPTION_HANDLER_DEFINITION(13)
EXCEPTION_HANDLER_DEFINITION(14)

EXCEPTION_HANDLER_DEFINITION(16)
EXCEPTION_HANDLER_DEFINITION(17)
EXCEPTION_HANDLER_DEFINITION(18)
EXCEPTION_HANDLER_DEFINITION(19)
EXCEPTION_HANDLER_DEFINITION(20)
EXCEPTION_HANDLER_DEFINITION(21)

EXCEPTION_HANDLER_DEFINITION(28)
EXCEPTION_HANDLER_DEFINITION(29)
EXCEPTION_HANDLER_DEFINITION(30)

IRQ_HANDLER_STUB(0)
IRQ_HANDLER_STUB(1)
IRQ_HANDLER_STUB(2)
IRQ_HANDLER_STUB(3)
IRQ_HANDLER_STUB(4)
IRQ_HANDLER_STUB(5)
IRQ_HANDLER_STUB(6)
IRQ_HANDLER_STUB(7)
IRQ_HANDLER_STUB(8)
IRQ_HANDLER_STUB(9)
IRQ_HANDLER_STUB(10)
IRQ_HANDLER_STUB(11)
IRQ_HANDLER_STUB(12)
IRQ_HANDLER_STUB(13)
IRQ_HANDLER_STUB(14)
IRQ_HANDLER_STUB(15)

static void irq_setup_stub_trap_handlers()
{
	trap_set_gate(0, EXCEPTION_HANDLER(0));
	trap_set_gate(1, EXCEPTION_HANDLER(1));
	trap_set_gate(2, EXCEPTION_HANDLER(2));
	trap_set_gate(3, EXCEPTION_HANDLER(3));
	trap_set_gate(4, EXCEPTION_HANDLER(4));
	trap_set_gate(5, EXCEPTION_HANDLER(5));
	trap_set_gate(6, EXCEPTION_HANDLER(6));
	trap_set_gate(7, EXCEPTION_HANDLER(7));
	trap_set_gate(8, EXCEPTION_HANDLER(8));
	trap_set_gate(9, EXCEPTION_HANDLER(9));
	trap_set_gate(10, EXCEPTION_HANDLER(10));
	trap_set_gate(11, EXCEPTION_HANDLER(11));
	trap_set_gate(12, EXCEPTION_HANDLER(12));
	trap_set_gate(13, EXCEPTION_HANDLER(13));
	trap_set_gate(14, EXCEPTION_HANDLER(14));
	trap_set_gate(16, EXCEPTION_HANDLER(16));
	trap_set_gate(17, EXCEPTION_HANDLER(17));
	trap_set_gate(18, EXCEPTION_HANDLER(18));
	trap_set_gate(19, EXCEPTION_HANDLER(19));
	trap_set_gate(20, EXCEPTION_HANDLER(20));
	trap_set_gate(21, EXCEPTION_HANDLER(21));
	trap_set_gate(28, EXCEPTION_HANDLER(28));
	trap_set_gate(29, EXCEPTION_HANDLER(29));
	trap_set_gate(30, EXCEPTION_HANDLER(30));

	trap_set_gate(32, IRQ_HANDLER(0));
	trap_set_gate(33, IRQ_HANDLER(1));
	trap_set_gate(34, IRQ_HANDLER(2));
	trap_set_gate(35, IRQ_HANDLER(3));
	trap_set_gate(36, IRQ_HANDLER(4));
	trap_set_gate(37, IRQ_HANDLER(5));
	trap_set_gate(38, IRQ_HANDLER(6));
	trap_set_gate(39, IRQ_HANDLER(7));
	trap_set_gate(40, IRQ_HANDLER(8));
	trap_set_gate(41, IRQ_HANDLER(9));
	trap_set_gate(42, IRQ_HANDLER(10));
	trap_set_gate(43, IRQ_HANDLER(11));
	trap_set_gate(44, IRQ_HANDLER(12));
	trap_set_gate(45, IRQ_HANDLER(13));
	trap_set_gate(46, IRQ_HANDLER(14));
	trap_set_gate(47, IRQ_HANDLER(15));
}

static void irq_remap(void)
{
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
}

/**
 * Set up an empty interrupt descriptor table
 * Expects that interrupts are disabled
 */
void trap_idt_setup(void)
{
	memset(&idt, 0, sizeof(idt));

	irq_setup_stub_trap_handlers();
	irq_remap();

	idtr.limit = sizeof(idt) - 1;
	idtr.base = idt;

	__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
}

inline void irq_enable(void)
{
	__asm__ __volatile__ ("sti");
}

inline void irq_disable(void)
{
	__asm__ __volatile__ ("cli");
}

void trap_set_gate(int n, void (*handler)(void *))
{
	idt[n].offset_low = (uint32_t)handler & 0xFFFF;
	idt[n].selector = 0x08;
	idt[n].zero = 0;
	idt[n].type_attr = 0x8F;
	idt[n].offset_high = ((uint32_t)handler >> 16) & 0xFFFF;
}
