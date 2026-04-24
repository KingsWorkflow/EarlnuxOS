; ============================================================================
;  EarlnuxOS - Interrupt Service Routine (ISR) Stubs
; kernel/arch/x86/isr.asm
; ============================================================================
; This file provides assembly stubs for all CPU exceptions and hardware IRQs.
; Each stub saves the CPU state, pushes interrupt number and error code,
; then jumps to the common ISR handler which dispatches to C handlers.

[bits 32]

section .text
global isr_stub_divide_error
global isr_stub_debug
global isr_stub_nmi
global isr_stub_breakpoint
global isr_stub_overflow
global isr_stub_bound_range
global isr_stub_invalid_opcode
global isr_stub_device_not_available
global isr_stub_double_fault
global isr_stub_coproc_segment_overrun
global isr_stub_invalid_tss
global isr_stub_segment_not_present
global isr_stub_stack_exception
global isr_stub_general_protection
global isr_stub_page_fault
global isr_stub_x87_floating_point
global isr_stub_alignment_check
global isr_stub_machine_check
global isr_stub_simd_exception

; IRQ stubs
global irq_stub_timer
global irq_stub_keyboard
global irq_stub_cascade
global irq_stub_serial2
global irq_stub_serial1
global irq_stub_parallel2
global irq_stub_floppy
global irq_stub_parallel1
global irq_stub_rtc
global irq_stub_irq9
global irq_stub_irq10
global irq_stub_irq11
global irq_stub_ps2_mouse
global irq_stub_fpu
global irq_stub_ata_primary
global irq_stub_ata_secondary
global irq_stub_spurious

extern isr_common_handler   ; defined in isr_common.c (or here)

; ==========================================================================
; Exception stubs without error code (CPU pushes no error code)
; We push a dummy error code (0) for uniformity
; ==========================================================================
%macro PUSH_NO_ERROR 1
    push dword 0              ; dummy error code
    push dword %1             ; interrupt number
    jmp isr_common_handler
%endmacro

isr_stub_divide_error:         PUSH_NO_ERROR 0
isr_stub_debug:                PUSH_NO_ERROR 1
isr_stub_nmi:                  PUSH_NO_ERROR 2
isr_stub_breakpoint:           PUSH_NO_ERROR 3
isr_stub_overflow:             PUSH_NO_ERROR 4
isr_stub_bound_range:          PUSH_NO_ERROR 5
isr_stub_invalid_opcode:       PUSH_NO_ERROR 6
isr_stub_device_not_available: PUSH_NO_ERROR 7
; ISR 8 (Double Fault) does have error code - handled separately
isr_stub_coproc_segment_overrun: PUSH_NO_ERROR 9
isr_stub_invalid_tss:          PUSH_NO_ERROR 10
isr_stub_segment_not_present:  PUSH_NO_ERROR 11
isr_stub_stack_exception:      PUSH_NO_ERROR 12
isr_stub_general_protection:   PUSH_NO_ERROR 13
; ISR 14 (Page Fault) has error code
isr_stub_x87_floating_point:   PUSH_NO_ERROR 16
isr_stub_alignment_check:      PUSH_NO_ERROR 17
isr_stub_machine_check:        PUSH_NO_ERROR 18
isr_stub_simd_exception:       PUSH_NO_ERROR 19

; ==========================================================================
; Exception stubs WITH error code (CPU pushes error code automatically)
; We just push interrupt number (error code already on stack)
; ==========================================================================
%macro PUSH_WITH_ERROR 1
    push dword %1             ; interrupt number (error code already pushed)
    jmp isr_common_handler
%endmacro

isr_stub_double_fault:    PUSH_WITH_ERROR 8
isr_stub_page_fault:      PUSH_WITH_ERROR 14

; ==========================================================================
; IRQ stubs - hardware IRQs (32-47) - all have no error code from CPU
; ==========================================================================
irq_stub_timer:         PUSH_NO_ERROR 32
irq_stub_keyboard:      PUSH_NO_ERROR 33
irq_stub_cascade:       PUSH_NO_ERROR 34
irq_stub_serial2:       PUSH_NO_ERROR 35
irq_stub_serial1:       PUSH_NO_ERROR 36
irq_stub_parallel2:     PUSH_NO_ERROR 37
irq_stub_floppy:        PUSH_NO_ERROR 38
irq_stub_parallel1:     PUSH_NO_ERROR 39
irq_stub_rtc:           PUSH_NO_ERROR 40
irq_stub_irq9:          PUSH_NO_ERROR 41
irq_stub_irq10:         PUSH_NO_ERROR 42
irq_stub_irq11:         PUSH_NO_ERROR 43
irq_stub_ps2_mouse:     PUSH_NO_ERROR 44
irq_stub_fpu:           PUSH_NO_ERROR 45
irq_stub_ata_primary:   PUSH_NO_ERROR 46
irq_stub_ata_secondary: PUSH_NO_ERROR 47
irq_stub_spurious:      PUSH_NO_ERROR 48  ; Not standard, but we have it
