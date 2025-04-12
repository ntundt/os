#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

struct interrupt_descriptor {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed));

struct idtr {
	uint16_t limit;
	struct interrupt_descriptor *base;
} __attribute__((packed));

void trap_idt_setup(void);

#define TRAP_EXCEPTION(n) (n)
#define TRAP_IRQ(n) (n+32)
void trap_set_gate(int n, void (*handler)(void *));

void irq_enable(void);
void irq_disable(void);

#endif
