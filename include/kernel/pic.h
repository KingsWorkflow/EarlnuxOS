#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H

#include <stdint.h>

// IRQ numbers
#define IRQ0  0
#define IRQ1  1
#define IRQ2  2
#define IRQ3  3
#define IRQ4  4
#define IRQ5  5
#define IRQ6  6
#define IRQ7  7
#define IRQ8  8
#define IRQ9  9
#define IRQ10 10
#define IRQ11 11
#define IRQ12 12
#define IRQ13 13
#define IRQ14 14
#define IRQ15 15

// PIC functions
void pic_remap(int offset1, int offset2);
void pic_unmask_irq(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_send_eoi(uint8_t irq);

#endif