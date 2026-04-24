; ============================================================================
;  EarlnuxOS - Kernel Entry (32-bit Protected Mode)
; kernel/entry.asm
; Linked to run at 0x00100000 (1MB physical / higher-half aware)
; ============================================================================

[BITS 32]
[SECTION .text.entry]

MULTIBOOT_MAGIC  equ 0x1BADB002
MULTIBOOT_FLAGS  equ 0x00000007  ; Request mem map + modules + video
MULTIBOOT_CHKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header must be in the first 8KB
ALIGN 4
multiboot_header:
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHKSUM
    ; address fields (used only if bit 16 set, not our case)
    dd 0, 0, 0, 0, 0
    ; video fields (request 80x25 text mode)
    dd 0    ; mode type: text
    dd 80   ; width
    dd 25   ; height
    dd 0    ; depth (0 = any for text)

EXTERN kernel_main
EXTERN __bss_start
EXTERN __bss_end
EXTERN __stack_top

; ============================================================================
; _start: Entered by either our stage2 or GRUB multiboot
; eax = MULTIBOOT_BOOTLOADER_MAGIC (0x2BADB002)
; ebx = pointer to multiboot info structure
; ============================================================================
GLOBAL _start
_start:
    ; Immediately save multiboot info
    mov  edi, eax   ; Multiboot magic
    mov  esi, ebx   ; Multiboot info pointer

    ; Set up kernel stack
    lea  esp, [__stack_top]
    xor  ebp, ebp

    ; Clear BSS segment
    lea  edi, [__bss_start]
    lea  ecx, [__bss_end]
    sub  ecx, edi
    shr  ecx, 2         ; Count in dwords
    xor  eax, eax
    rep stosd

    ; Restore multiboot args for kernel_main call
    push esi            ; arg2: multiboot info ptr
    push edi            ; arg1: multiboot magic

    ; Call kernel C entry point
    ; void kernel_main(uint32_t mb_magic, multiboot_info_t *mb_info)
    call kernel_main

    ; kernel_main is NORETURN — if it returns, halt
.halt:
    cli
    hlt
    jmp  .halt

; ============================================================================
; Section declarations for linker
; ============================================================================
[SECTION .bss]
ALIGN 16
kernel_stack:    resb 65536      ; 64 KB kernel stack
GLOBAL __stack_top
__stack_top:
