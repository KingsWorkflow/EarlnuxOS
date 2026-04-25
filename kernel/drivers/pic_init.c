#include <kernel/kernel.h>
#include <kernel/pic.h>

void pic_init(void) {
    // Remap PIC interrupts to avoid conflicts with CPU exceptions
    pic_remap(0x20, 0x28);
    KINFO("PIC", "8259 PIC initialized\n");
}