[BITS 32]

section .text

; gdt_flush(uint32_t gdt_ptr_addr)
global gdt_flush
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    ; Reload segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS
    jmp 0x08:.flush
.flush:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits