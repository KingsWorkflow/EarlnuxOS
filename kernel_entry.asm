; kernel_entry.asm - Kernel entry point (32-bit protected mode)
; Assembled with: nasm -f elf32 kernel_entry.asm -o kernel_entry.o

[BITS 32]

extern main
extern KERNEL_BASE

; Multiboot header (optional, for GRUB compatibility)
MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_FLAGS equ 0x00000000
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .text
    ; Multiboot header
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

global kernel_entry
kernel_entry:
    ; Set up stack
    mov esp, 0x9F000       ; Stack pointer (grows downward)
    and esp, 0xFFFFFFF0    ; 16-byte align
    sub esp, 4             ; Reserve space for return address

    ; Call main C function
    call main

    ; Hang if main returns
    hlt
    jmp $

section .bss
    ; Uninitialized data (BSS section)
    align 4096
    kernel_stack: resb 4096