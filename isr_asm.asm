; isr_asm.asm - Interrupt handlers in assembly

[BITS 32]

extern keyboard_handler

global isr33

isr33:
    ; Save registers
    pusha
    
    ; Read scancode from keyboard
    mov al, 0x60        ; PS/2 keyboard data port
    in al, dx           ; Read scancode
    
    ; Call keyboard handler
    movzx eax, al
    push eax
    call keyboard_handler
    add esp, 4

    ; Acknowledge interrupt to PIC
    mov al, 0x20        ; EOI (End of Interrupt)
    out 0x20, al        ; Master PIC

    ; Restore registers
    popa
    iret                ; Return from interrupt