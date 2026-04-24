/* ============================================================================
 *  EarlnuxOS - Programmable Interrupt Controller (PIC) Driver
 * kernel/drivers/pic.c
 * ============================================================================ */

#include <arch/x86/ports.h>
#include <kernel/kernel.h>

/* PIC1 and PIC2 command and data ports */
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

/* Initialize PIC (8259A) and remap IRQs to interrupt vectors 0x20-0x2F */
void pic_init(void) {
    /* Save masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Begin initialization sequence */
    outb(PIC1_CMD, 0x11);   /* ICW1: Initialize, edge-triggered, cascade */
    outb(PIC2_CMD, 0x11);
    io_wait();

    /* ICW2: Vector offsets (map IRQ0-7 to 0x20-0x27, IRQ8-15 to 0x28-0x2F) */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    io_wait();

    /* ICW3: Tell master PIC about slave at IRQ2 (0000 0100) */
    outb(PIC1_DATA, 0x04);
    /* ICW3: Tell slave PIC its cascade identity (2) */
    outb(PIC2_DATA, 0x02);
    io_wait();

    /* ICW4: 8086 mode, not auto-EOI, not buffered, normal EOI */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Restore saved masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/* Send End-of-Interrupt (EOI) to PIC(s) */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);   /* EOI to slave PIC */
    }
    outb(PIC1_CMD, 0x20);       /* EOI to master PIC */
}

/* Mask/ unmask a specific IRQ */
void pic_mask_irq(uint8_t irq) {
    if (irq < 8) {
        uint8_t mask = inb(PIC1_DATA);
        mask |= (1 << irq);
        outb(PIC1_DATA, mask);
    } else {
        uint8_t mask = inb(PIC2_DATA);
        mask |= (1 << (irq - 8));
        outb(PIC2_DATA, mask);
    }
}

void pic_unmask_irq(uint8_t irq) {
    if (irq < 8) {
        uint8_t mask = inb(PIC1_DATA);
        mask &= ~(1 << irq);
        outb(PIC1_DATA, mask);
    } else {
        uint8_t mask = inb(PIC2_DATA);
        mask &= ~(1 << (irq - 8));
        outb(PIC2_DATA, mask);
    }
}
