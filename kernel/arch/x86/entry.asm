[BITS 32]

; Multiboot header constants
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000000
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

section .text

; Kernel entry point
global _start
_start:
    ; Set up stack
    mov esp, stack_top

    ; Clear BSS section
    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Push multiboot arguments
    push ebx
    push eax

    ; Call kernel main
    extern kernel_main
    call kernel_main

    ; Hang if kernel returns
hang:
    cli
    hlt
    jmp hang

section .bss
stack_bottom:
    resb 16384 ; 16KB stack
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits