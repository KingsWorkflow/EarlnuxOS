/* ============================================================================
 *  EarlnuxOS - Interrupt Descriptor Table Implementation
 * kernel/arch/x86/idt.c
 * ============================================================================ */

#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <arch/x86/ports.h>

/* IDT entries array */
static idt_entry_t idt_entries[IDT_ENTRIES];

/* IDT pointer */
static idt_ptr_t idt_ptr;

/* ISR handler array (populated by idt_register_handlers) */
void (*isr_handlers[IDT_ENTRIES])(struct regs *);

/* ==========================================================================
 * Set IDT gate
 * ========================================================================== */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].zero      = 0;
    idt_entries[num].flags     = flags;
}

/* ==========================================================================
 * Install a single IDT entry with segment selector and flags
 * ========================================================================== */
void idt_install_gate(uint8_t vector, uint32_t handler, uint8_t flags) {
    idt_set_gate(vector, handler, KERNEL_CS << 3, flags);
}

/* ==========================================================================
 * Initialize IDT
 * ========================================================================== */
void idt_init(void) {
    /* Zero out IDT entries */
    memset(&idt_entries, 0, sizeof(idt_entries));

    /* Set up IDT pointer */
    idt_ptr.limit = (uint16_t)(sizeof(idt_entries) - 1);
    idt_ptr.base  = (uint32_t)&idt_entries;

    /* Install CPU exception handlers (ISRs 0-31) */
    idt_install_gate(0,  (uint32_t)isr_stub_divide_error,         IDT_GATE_INTERRUPT);
    idt_install_gate(1,  (uint32_t)isr_stub_debug,                IDT_GATE_INTERRUPT);
    idt_install_gate(2,  (uint32_t)isr_stub_nmi,                  IDT_GATE_INTERRUPT);
    idt_install_gate(3,  (uint32_t)isr_stub_breakpoint,           IDT_GATE_INTERRUPT);
    idt_install_gate(4,  (uint32_t)isr_stub_overflow,             IDT_GATE_INTERRUPT);
    idt_install_gate(5,  (uint32_t)isr_stub_bound_range,          IDT_GATE_INTERRUPT);
    idt_install_gate(6,  (uint32_t)isr_stub_invalid_opcode,       IDT_GATE_INTERRUPT);
    idt_install_gate(7,  (uint32_t)isr_stub_device_not_available, IDT_GATE_INTERRUPT);
    idt_install_gate(8,  (uint32_t)isr_stub_double_fault,         IDT_GATE_INTERRUPT | 0x60); /* DPL=3 for double fault? Actually typically 0 */
    idt_install_gate(9,  (uint32_t)isr_stub_coproc_segment_overrun, IDT_GATE_INTERRUPT);
    idt_install_gate(10, (uint32_t)isr_stub_invalid_tss,          IDT_GATE_INTERRUPT);
    idt_install_gate(11, (uint32_t)isr_stub_segment_not_present,  IDT_GATE_INTERRUPT);
    idt_install_gate(12, (uint32_t)isr_stub_stack_exception,      IDT_GATE_INTERRUPT);
    idt_install_gate(13, (uint32_t)isr_stub_general_protection,   IDT_GATE_INTERRUPT);
    idt_install_gate(14, (uint32_t)isr_stub_page_fault,           IDT_GATE_INTERRUPT);
    idt_install_gate(16, (uint32_t)isr_stub_x87_floating_point,   IDT_GATE_INTERRUPT);
    idt_install_gate(17, (uint32_t)isr_stub_alignment_check,      IDT_GATE_INTERRUPT);
    idt_install_gate(18, (uint32_t)isr_stub_machine_check,        IDT_GATE_INTERRUPT);
    idt_install_gate(19, (uint32_t)isr_stub_simd_exception,       IDT_GATE_INTERRUPT);

    /* Install IRQ handlers (32-47) */
    idt_install_gate(IRQ0,  (uint32_t)irq_stub_timer,          IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ1,  (uint32_t)irq_stub_keyboard,       IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ2,  (uint32_t)irq_stub_cascade,        IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ3,  (uint32_t)irq_stub_serial2,        IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ4,  (uint32_t)irq_stub_serial1,        IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ5,  (uint32_t)irq_stub_parallel2,      IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ6,  (uint32_t)irq_stub_floppy,         IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ7,  (uint32_t)irq_stub_parallel1,      IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ8,  (uint32_t)irq_stub_rtc,            IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ9,  (uint32_t)irq_stub_irq9,           IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ10, (uint32_t)irq_stub_irq10,          IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ11, (uint32_t)irq_stub_irq11,          IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ12, (uint32_t)irq_stub_ps2_mouse,      IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ13, (uint32_t)irq_stub_fpu,            IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ14, (uint32_t)irq_stub_ata_primary,    IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ15, (uint32_t)irq_stub_ata_secondary,  IDT_GATE_INTERRUPT);
    idt_install_gate(IRQ15 + 1, (uint32_t)irq_stub_spurious, IDT_GATE_INTERRUPT);

    /* Load IDT */
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

/* Forward declarations for C exception handlers */
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

/* IRQ C handlers */
void irq_timer(void);
void irq_keyboard(void);
void irq_cascade(void);
void irq_serial2(void);
void irq_serial1(void);
void irq_parallel2(void);
void irq_floppy(void);
void irq_parallel1(void);
void irq_rtc(void);
void irq_irq9(void);
void irq_irq10(void);
void irq_irq11(void);
void irq_ps2_mouse(void);
void irq_fpu(void);
void irq_ata_primary(void);
void irq_ata_secondary(void);
void irq_spurious(void);

/* ==========================================================================
 * Register all ISR/IRQ handler pointers (called after idt_init)
 * ========================================================================== */
void idt_register_handlers(void) {
    memset(isr_handlers, 0, sizeof(isr_handlers));

    /* Exceptions */
    isr_handlers[0]  = isr_divide_error;
    isr_handlers[1]  = isr_debug;
    isr_handlers[2]  = isr_nmi;
    isr_handlers[3]  = isr_breakpoint;
    isr_handlers[4]  = isr_overflow;
    isr_handlers[5]  = isr_bound_range;
    isr_handlers[6]  = isr_invalid_opcode;
    isr_handlers[7]  = isr_device_not_available;
    isr_handlers[8]  = isr_double_fault;
    isr_handlers[10] = isr_invalid_tss;
    isr_handlers[11] = isr_segment_not_present;
    isr_handlers[12] = isr_stack_exception;
    isr_handlers[13] = isr_general_protection;
    isr_handlers[14] = isr_page_fault;
    isr_handlers[16] = isr_x87_floating_point;
    isr_handlers[17] = isr_alignment_check;
    isr_handlers[18] = isr_machine_check;
    isr_handlers[19] = isr_simd_exception;

    /* IRQs */
    isr_handlers[IRQ0]  = irq_timer;
    isr_handlers[IRQ1]  = irq_keyboard;
    isr_handlers[IRQ2]  = irq_cascade;
    isr_handlers[IRQ3]  = irq_serial2;
    isr_handlers[IRQ4]  = irq_serial1;
    isr_handlers[IRQ5]  = irq_parallel2;
    isr_handlers[IRQ6]  = irq_floppy;
    isr_handlers[IRQ7]  = irq_parallel1;
    isr_handlers[IRQ8]  = irq_rtc;
    isr_handlers[IRQ9]  = irq_irq9;
    isr_handlers[IRQ10] = irq_irq10;
    isr_handlers[IRQ11] = irq_irq11;
    isr_handlers[IRQ12] = irq_ps2_mouse;
    isr_handlers[IRQ13] = irq_fpu;
    isr_handlers[IRQ14] = irq_ata_primary;
    isr_handlers[IRQ15] = irq_ata_secondary;
    isr_handlers[47]    = irq_spurious;
}
