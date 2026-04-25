#include "idt.h"
#include <kernel/regs.h>
#include <kernel/kernel.h>

// Array of ISR handlers
void (*isr_handlers[IDT_ENTRIES])(struct regs *) = {0};

// Register an ISR handler
void isr_register_handler(int irq, void (*handler)(struct regs *)) {
    if (irq < IDT_ENTRIES) {
        isr_handlers[irq] = handler;
        KINFO("ISR", "Handler registered");
    }
}

// Default handler
void isr_default_handler(struct regs *r) {
    (void)r;  // Suppress unused parameter warning
    KINFO("ISR", "Unhandled interrupt");
}