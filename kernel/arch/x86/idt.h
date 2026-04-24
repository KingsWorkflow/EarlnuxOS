/* ============================================================================
 *  EarlnuxOS - Interrupt Descriptor Table (IDT)
 * kernel/arch/x86/idt.h
 * ============================================================================ */

#ifndef  EarlnuxOS_ARCH_X86_IDT_H
#define  EarlnuxOS_ARCH_X86_IDT_H

#include <types.h>

/* IDT entry count: 256 vectors (0-255) */
#define IDT_ENTRIES      256

/* IRQ remapping offset (after CPU exceptions) */
#define IRQ0             32  /* Timer */
#define IRQ1             33  /* Keyboard */
#define IRQ2             34  /* Cascade (not used) */
#define IRQ3             35  /* Serial port 2 */
#define IRQ4             36  /* Serial port 1 */
#define IRQ5             37  /* Parallel port 2 */
#define IRQ6             38  /* Floppy disk */
#define IRQ7             39  /* Parallel port 1 */
#define IRQ8             40  /* CMOS real-time clock */
#define IRQ9             41  /* Free for peripherals */
#define IRQ10            42  /* Free for peripherals */
#define IRQ11            43  /* Free for peripherals */
#define IRQ12            44  /* PS/2 mouse */
#define IRQ13            45  /* FPU/coprocessor */
#define IRQ14            46  /* Primary ATA hard disk */
#define IRQ15            47  /* Secondary ATA hard disk */

/* IDT gate types */
#define IDT_GATE_TYPE(x) ((x) & 0x0F)

/* 32-bit interrupt gate (present, DPL=0, storage segment, 32-bit) */
#define IDT_GATE_INTERRUPT 0x8E
/* 32-bit trap gate (present, DPL=0, storage segment, 32-bit, NO IF clear) */
#define IDT_GATE_TRAP     0xEF
/* 32-bit task gate (present, DPL=0) */
#define IDT_GATE_TASK     0x85

/* IDT entry structure (8 bytes) */
typedef struct PACKED {
    uint16_t base_low;     /* Lower 16 bits of ISR address */
    uint16_t sel;          /* Kernel segment selector */
    uint8_t  zero;         /* Reserved (always 0) */
    uint8_t  flags;        /* Type, attributes */
    uint16_t base_high;    /* Upper 16 bits of ISR address */
} idt_entry_t;

/* IDT pointer structure (6 bytes) */
typedef struct PACKED {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

/* Array of ISR handler function pointers (defined in isr.c) */
extern void (*isr_handlers[IDT_ENTRIES])(struct regs *);

/* ========================================================================== */
/* Register State Structure (pushed by common handler) */
/* ========================================================================== */
typedef struct regs {
    uint32_t ds;      /* Data segment selector (pushed by common handler) */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t int_no;  /* Interrupt number */
    uint32_t err_code;/* Error code (if applicable) */
    uint32_t eip;     /* Instruction pointer */
    uint32_t cs;      /* Code segment */
    uint32_t eflags;  /* Flags */
    uint32_t user_esp;/* User stack pointer (if privilege change) */
    uint32_t user_ss; /* User stack segment (if privilege change) */
} regs_t;

/* Initialize and install IDT */
void idt_init(void);

/* Register ISR/IRQ handler functions */
void idt_register_handlers(void);

/* Set individual IDT gate */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install_gate(uint8_t vector, uint32_t handler, uint8_t flags);

/* ========================================================================== */
/* Assembly stubs (provided in isr.asm and isr_common.asm) */
/* ========================================================================== */
extern void isr_common_handler(void);
extern void isr_stub_divide_error(void);
extern void isr_stub_debug(void);
extern void isr_stub_nmi(void);
extern void isr_stub_breakpoint(void);
extern void isr_stub_overflow(void);
extern void isr_stub_bound_range(void);
extern void isr_stub_invalid_opcode(void);
extern void isr_stub_device_not_available(void);
extern void isr_stub_double_fault(void);
extern void isr_stub_coproc_segment_overrun(void);
extern void isr_stub_invalid_tss(void);
extern void isr_stub_segment_not_present(void);
extern void isr_stub_stack_exception(void);
extern void isr_stub_general_protection(void);
extern void isr_stub_page_fault(void);
extern void isr_stub_x87_floating_point(void);
extern void isr_stub_alignment_check(void);
extern void isr_stub_machine_check(void);
extern void isr_stub_simd_exception(void);

/* IRQ handlers (soft IRQs, not exceptions) */
extern void irq_stub_timer(void);
extern void irq_stub_keyboard(void);
extern void irq_stub_cascade(void);
extern void irq_stub_serial2(void);
extern void irq_stub_serial1(void);
extern void irq_stub_parallel2(void);
extern void irq_stub_floppy(void);
extern void irq_stub_parallel1(void);
extern void irq_stub_rtc(void);
extern void irq_stub_irq9(void);
extern void irq_stub_irq10(void);
extern void irq_stub_irq11(void);
extern void irq_stub_ps2_mouse(void);
extern void irq_stub_fpu(void);
extern void irq_stub_ata_primary(void);
extern void irq_stub_ata_secondary(void);
extern void irq_stub_spurious(void);

/* Register an ISR handler */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif /*  EarlnuxOS_ARCH_X86_IDT_H */
