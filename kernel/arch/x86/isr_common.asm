; ============================================================================
;  EarlnuxOS - Common Interrupt Handler (called by all ISR/IRQ stubs)
; kernel/arch/x86/isr_common.asm
; ============================================================================
; Stack layout upon entry from stub:
;   [esp]     = return address (from call/jmp)
;   [esp+4]   = int_no
;   [esp+8]   = err_code (or dummy 0)
;   [esp+12]  = eip  (pushed by CPU)
;   [esp+16]  = cs   (pushed by CPU)
;   [esp+20]  = eflags (pushed by CPU)
; We push segment registers and general-purpose registers to build regs struct.
; Then call C function: void isr_dispatch(struct regs *r);
; After return, restore registers and iret.

[bits 32]

section .text
global isr_common_handler
extern isr_dispatch

isr_common_handler:
    ; Save all general-purpose registers and segment registers
    pusha                       ; pushes EAX, ECX, EDX, EBX, ESP(orig), EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs

    ; Load the kernel data segment descriptor into segment registers
    mov ax, KERNEL_DS << 3      ; kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push pointer to the current stack (which points to regs) as argument
    mov eax, esp
    push eax
    call isr_dispatch
    add esp, 4                  ; clean up argument

    ; Restore segment registers and general registers
    pop gs
    pop fs
    pop es
    pop ds
    popa

    ; Remove int_no, err_code, eip, cs, eflags from stack (20 bytes)
    add esp, 20

    iret
