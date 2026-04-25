; ============================================================================
;  EarlnuxOS - Interrupt Service Routine Stubs
; kernel/arch/x86/isr_common.asm
; ============================================================================

[bits 32]

; External C handlers
extern isr_handler
extern irq_handler

; Macro for Exceptions (no error code)
%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

; Macro for Exceptions (with error code)
%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push dword %1
    jmp isr_common_stub
%endmacro

; Macro for Hardware IRQs
%macro IRQ 2
global irq%1
irq%1:
    cli
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

; Standard ISR Stub
isr_common_stub:
    pusha                    ; Push EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX

    mov ax, ds               ; Lower 16-bits of eax = ds
    push eax                 ; Save the data segment descriptor

    mov ax, 0x10             ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                 ; Push a pointer to the registers (struct regs *)
    call isr_handler
    add esp, 4               ; Clean up the stack

    pop eax                  ; Restore the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pop EDI, ESI, EBP, etc
    add esp, 8               ; Cleans up the pushed error code and pushed ISR number
    iret                     ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Hardware IRQ Stub
irq_common_stub:
    pusha                    ; Push EDI, ESI, EBP, etc

    mov ax, ds               ; Save data segment
    push eax

    mov ax, 0x10             ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                 ; Pointer to regs
    call irq_handler
    add esp, 4

    pop eax                  ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret                     ; iret will restore the IF (interrupt flag) automatically

; Define all 32 Exceptions
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

; Define 16 Hardware IRQs (32-47)
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

section .note.GNU-stack noalloc noexec nowrite progbits