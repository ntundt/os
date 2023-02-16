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

void setup_idt(void);
void setup_interrupt_gate(int n, void (*handler)(void *));

void enable_interrupts(void);
void disable_interrupts(void);

#endif
