// idt.c - Interrupt Descriptor Table setup

#include <stdint.h>

#define IDT_ENTRIES 256
#define IDT_BASE 0x00000000

typedef struct {
    uint16_t offset_1;      // Offset bits 0-15
    uint16_t selector;      // Code segment selector
    uint8_t  zero;          // Must be zero
    uint8_t  type_attr;     // Type and attributes
    uint16_t offset_2;      // Offset bits 16-31
} __attribute__((packed)) IDT_Entry;

typedef struct {
    uint16_t size;
    uint32_t base;
} __attribute__((packed)) IDT_Pointer;

static IDT_Entry idt[IDT_ENTRIES];
static IDT_Pointer idt_ptr;

extern void isr0(void);   // Defined in isr_asm.asm
extern void isr33(void);  // IRQ1 (keyboard)

void idt_init(void) {
    idt_ptr.base = (uint32_t)&idt;
    idt_ptr.size = sizeof(idt) - 1;

    // Zero out IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_1 = 0;
        idt[i].selector = 0x08;  // Code segment
        idt[i].zero = 0;
        idt[i].type_attr = 0x00;
        idt[i].offset_2 = 0;
    }

    // Set up keyboard interrupt (IRQ1 = INT 33)
    uint32_t handler = (uint32_t)&isr33;
    idt[33].offset_1 = handler & 0xFFFF;
    idt[33].offset_2 = (handler >> 16) & 0xFFFF;
    idt[33].selector = 0x08;
    idt[33].zero = 0;
    idt[33].type_attr = 0x8E;  // Present, 32-bit interrupt gate

    // Load IDT
    asm("lidt (%0)" : : "r" (&idt_ptr));

    // Enable IRQ1 (PIC configuration)
    // Mask all interrupts except IRQ1
    outb(0x21, 0xFD);  // Master PIC: unmask IRQ1
}

void outb(uint16_t port, uint8_t value) {
    asm("outb %0, %1" : : "a" (value), "Nd" (port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    asm("inb %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}