/* ============================================================================
 *  EarlnuxOS - ISR Dispatch and Exception Handlers
 * kernel/arch/x86/isr.c
 * ============================================================================ */

#include <arch/x86/idt.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <types.h>

/* ISR handler pointer array (defined in idt.c) */
extern void (*isr_handlers[IDT_ENTRIES])(struct regs *);

/* Forward declarations for all IRQ/ISR handlers (defined elsewhere) */
void isr_divide_error(struct regs *r);
void isr_debug(struct regs *r);
void isr_nmi(struct regs *r);
void isr_breakpoint(struct regs *r);
void isr_overflow(struct regs *r);
void isr_bound_range(struct regs *r);
void isr_invalid_opcode(struct regs *r);
void isr_device_not_available(struct regs *r);
void isr_double_fault(struct regs *r);
void isr_coproc_segment_overrun(struct regs *r);
void isr_invalid_tss(struct regs *r);
void isr_segment_not_present(struct regs *r);
void isr_stack_exception(struct regs *r);
void isr_general_protection(struct regs *r);
void isr_page_fault(struct regs *r);
void isr_x87_floating_point(struct regs *r);
void isr_alignment_check(struct regs *r);
void isr_machine_check(struct regs *r);
void isr_simd_exception(struct regs *r);

/* IRQ handlers */
void irq_timer(struct regs *r);
void irq_keyboard(struct regs *r);
void irq_cascade(struct regs *r);
void irq_serial2(struct regs *r);
void irq_serial1(struct regs *r);
void irq_parallel2(struct regs *r);
void irq_floppy(struct regs *r);
void irq_parallel1(struct regs *r);
void irq_rtc(struct regs *r);
void irq_irq9(struct regs *r);
void irq_irq10(struct regs *r);
void irq_irq11(struct regs *r);
void irq_ps2_mouse(struct regs *r);
void irq_fpu(struct regs *r);
void irq_ata_primary(struct regs *r);
void irq_ata_secondary(struct regs *r);
void irq_spurious(struct regs *r);

/* ISR handler pointers (set by idt_register_handlers) */
void (*isr_handlers[IDT_ENTRIES])(struct regs *) = {0};

/* ==========================================================================
 * isr_dispatch - Common interrupt dispatcher
 * Called from isr_common_handler (assembly) after saving context
 * ========================================================================== */
void isr_dispatch(struct regs *r) {
    uint8_t int_no = r->int_no;

    if (int_no < IDT_ENTRIES && isr_handlers[int_no]) {
        isr_handlers[int_no](r);
    } else {
        /* Unknown interrupt - print info and halt */
        console_set_color(VGA_ERROR_ATTR);
        kprintf("\n\n!! Unhandled interrupt %u !!\n", int_no);
        console_set_color(VGA_DEFAULT_ATTR);
        /* Dump registers in a panic */
        panic("Unhandled interrupt");
    }
}

/* ==========================================================================
 * Exception Handlers - print error info and halt
 * ========================================================================== */

#define EXC_HANDLER(name, msg) void name(struct regs *r) { \
    console_set_color(VGA_ERROR_ATTR); \
    kprintf("\n\nException %s (%u) at eip=0x%x\n", msg, r->int_no, r->eip); \
    console_set_color(VGA_DEFAULT_ATTR); \
    panic(msg); \
}

/* For exceptions with error code, we might want to display it */
#define EXC_HANDLER_EC(name, msg) void name(struct regs *r) { \
    console_set_color(VGA_ERROR_ATTR); \
    kprintf("\n\nException %s (%u) err=0x%x at eip=0x%x\n", msg, r->int_no, r->err_code, r->eip); \
    console_set_color(VGA_DEFAULT_ATTR); \
    panic(msg); \
}

/* Define handlers */
EXC_HANDLER(isr_divide_error,            "Divide Error")
EXC_HANDLER(isr_debug,                   "Debug")
EXC_HANDLER(isr_nmi,                     "NMI")
/* Breakpoint - we can ignore for debugging */
void isr_breakpoint(struct regs *r) {
    kprintf("\nBreakpoint hit at eip=0x%x\n", r->eip);
    /* Could invoke debugger here */
}
EXC_HANDLER(isr_overflow,                "Overflow")
EXC_HANDLER(isr_bound_range,             "Bound Range Exceeded")
EXC_HANDLER(isr_invalid_opcode,          "Invalid Opcode")
EXC_HANDLER(isr_device_not_available,    "Device Not Available")
EXC_HANDLER_EC(isr_double_fault,         "Double Fault")
EXC_HANDLER(isr_coproc_segment_overrun,  "Coprocessor Segment Overrun")
EXC_HANDLER_EC(isr_invalid_tss,          "Invalid TSS")
EXC_HANDLER_EC(isr_segment_not_present,  "Segment Not Present")
EXC_HANDLER_EC(isr_stack_exception,      "Stack Exception")
EXC_HANDLER_EC(isr_general_protection,   "General Protection Fault")
EXC_HANDLER_EC(isr_page_fault,           "Page Fault")
EXC_HANDLER(isr_x87_floating_point,      "x87 FPU Floating-Point Error")
EXC_HANDLER(isr_alignment_check,         "Alignment Check")
EXC_HANDLER(isr_machine_check,           "Machine Check")
EXC_HANDLER(isr_simd_exception,          "SIMD Exception")

/* ==========================================================================
 * IRQ Handlers - stub implementations (real work in drivers)
 * ========================================================================== */

void irq_timer(struct regs *r) {
    (void)r; /* suppress unused warning */
    /* EOI to PIC */
    outb(0x20, 0x20);
    /* Timer tick - increment tick count */
    extern volatile uint32_t system_ticks;
    system_ticks++;
    /* Could call timer_tick() callback list */
}

void irq_keyboard(struct regs *r) {
    (void)r;
    /* Read scancode from keyboard controller */
    uint8_t scancode = inb(0x60);
    /* Pass to keyboard driver */
    extern void keyboard_handle_scancode(uint8_t sc);
    keyboard_handle_scancode(scancode);
    /* Send EOI */
    outb(0x20, 0x20);
}

void irq_cascade(struct regs *r) { (void)r; outb(0x20, 0x20); }
void irq_serial2(struct regs *r)  { (void)r; /* handle serial2 */ outb(0x20, 0x20); }
void irq_serial1(struct regs *r)  { (void)r; /* handle serial1 */ outb(0x20, 0x20); }
void irq_parallel2(struct regs *r){ (void)r; /* handle parallel2 */ outb(0x20, 0x20); }
void irq_floppy(struct regs *r)   { (void)r; outb(0x20, 0x20); }
void irq_parallel1(struct regs *r){ (void)r; outb(0x20, 0x20); }
void irq_rtc(struct regs *r)      { (void)r; /* RTC tick */ outb(0x20, 0x20); }
void irq_irq9(struct regs *r)     { (void)r; outb(0x20, 0x20); }
void irq_irq10(struct regs *r)    { (void)r; outb(0x20, 0x20); }
void irq_irq11(struct regs *r)    { (void)r; outb(0x20, 0x20); }
void irq_ps2_mouse(struct regs *r){ (void)r; /* mouse */ outb(0x20, 0x20); }
void irq_fpu(struct regs *r)      { (void)r; outb(0x20, 0x20); }
void irq_ata_primary(struct regs *r){ (void)r; /* IDE primary */ outb(0x20, 0x20); }
void irq_ata_secondary(struct regs *r){ (void)r; /* IDE secondary */ outb(0x20, 0x20); }
void irq_spurious(struct regs *r) { (void)r; /* spurious IRQ7 */ outb(0x20, 0x20); }
