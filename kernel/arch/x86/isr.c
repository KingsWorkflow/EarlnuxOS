#include <kernel/regs.h>
#include <kernel/kernel.h>
#include <kernel/pic.h>
#include "idt.h"

// External handler array
extern void (*isr_handlers[IDT_ENTRIES])(struct regs *);

// ISR handler function (called from assembly)
void isr_handler(struct regs *r) {
    if (isr_handlers[r->int_no]) {
        isr_handlers[r->int_no](r);
    } else {
        KINFO("ISR", "Unhandled exception occurred");
    }
}

// IRQ handler function (called from assembly)
void irq_handler(struct regs *r) {
    /* Visual indicator: flash a character in the top right corner */
    volatile char *vga = (volatile char*)0xB8000;
    static uint8_t flash = 0;
    vga[158] = (flash++ % 2) ? '*' : ' ';
    vga[159] = 0x0E; /* Yellow */

    /* Send EOI to PIC */
    if (r->int_no >= 32 && r->int_no <= 47) {
        pic_send_eoi(r->int_no - 32);
    }

    /* Safety check: ensure handler is valid and not NULL */
    if (r->int_no < IDT_ENTRIES && isr_handlers[r->int_no]) {
        /* Call the actual handler */
        void (*handler)(struct regs *) = isr_handlers[r->int_no];
        if ((uint32_t)handler > 0x1000) { // Basic sanity check for kernel address
            handler(r);
        }
    }
}

void isr_init(void) {
    KINFO("ISR", "Interrupt Service Routines initialized\n");
}