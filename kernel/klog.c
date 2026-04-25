#include <kernel/kernel.h>
#include <stdarg.h>

// Simple logging function
void klog(int level, const char *module, const char *fmt, ...) {
    const char *level_str;

    switch(level) {
        case LOG_INFO:  level_str = "INFO";  break;
        case LOG_WARN:  level_str = "WARN";  break;
        case LOG_ERROR: level_str = "ERROR"; break;
        default:        level_str = "DEBUG"; break;
    }

    kprintf("[%s] %s: ", level_str, module);

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);

    kprintf("\n");
}

void panic(const char *msg) {
    /* Write directly to VGA in case kprintf is broken */
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    const char *p = "PANIC: ";
    for (int i = 0; *p; i++, p++) vga[i] = (uint16_t)*p | 0x4F00; /* white on red */

    kprintf("\n\n*** KERNEL PANIC: %s ***\n", msg);
    kprintf("System halted.\n");

    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}