/* ============================================================================
 *  EarlnuxOS - Programmable Interrupt Controller (PIC) Driver
 * kernel/drivers/pic.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <kernel/pic.h>
#include <stdint.h>

/* PIC1 and PIC2 command and data ports */
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

void pic_remap(int offset1, int offset2) {
    /* ICW1: Start initialization in cascade mode */
    outb(PIC1_CMD, 0x11);
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();
    
    /* ICW2: Master PIC vector offset */
    outb(PIC1_DATA, offset1);
    io_wait();
    /* ICW2: Slave PIC vector offset */
    outb(PIC2_DATA, offset2);
    io_wait();
    
    /* ICW3: Tell Master PIC there is a slave PIC at IRQ2 (0000 0100) */
    outb(PIC1_DATA, 0x04);
    io_wait();
    /* ICW3: Tell Slave PIC its cascade identity (0000 0010) */
    outb(PIC2_DATA, 0x02);
    io_wait();
    
    /* ICW4: Set 8086/88 mode */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();
    
    /* Mask all interrupts on both PICs except cascade (IRQ2) */
    outb(PIC1_DATA, 0xFB); /* 1111 1011 */
    outb(PIC2_DATA, 0xFF); /* 1111 1111 */
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
