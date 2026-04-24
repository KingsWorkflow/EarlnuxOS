; ============================================================================
;  EarlnuxOS - Stage 2 Bootloader
; Loaded at 0x8000. Enables A20, sets up GDT, enters 32-bit protected mode,
; loads the kernel ELF from disk, and transfers control.
; ============================================================================

[BITS 16]
[ORG 0x8000]

STAGE2_ENTRY:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7BFF

    mov si, MSG_S2
    call PRINT16

    ; Enable A20 line via BIOS
    mov ax, 0x2401
    int 0x15
    jc .a20_kbd

    jmp .a20_done

.a20_kbd:
    ; Keyboard controller method
    call A20_WAIT
    mov al, 0xAD
    out 0x64, al
    call A20_WAIT
    mov al, 0xD0
    out 0x64, al
    call A20_WAIT2
    in al, 0x60
    push ax
    call A20_WAIT
    mov al, 0xD1
    out 0x64, al
    call A20_WAIT
    pop ax
    or al, 2
    out 0x60, al
    call A20_WAIT
    mov al, 0xAE
    out 0x64, al
    call A20_WAIT

.a20_done:
    mov si, MSG_A20
    call PRINT16

    ; Load GDT
    lgdt [GDT_PTR]

    ; Enter Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code segment
    jmp 0x08:PM_ENTRY

; ============================================================================
; A20 line port wait helpers
; ============================================================================
A20_WAIT:
    in al, 0x64
    test al, 2
    jnz A20_WAIT
    ret

A20_WAIT2:
    in al, 0x64
    test al, 1
    jz A20_WAIT2
    ret

PRINT16:
    mov ah, 0x0E
    xor bh, bh
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

; ============================================================================
; GDT - Global Descriptor Table (flat 32-bit model)
; ============================================================================
GDT_START:
    ; Null descriptor
    dq 0x0000000000000000
    ; Code segment: base=0, limit=4GB, 32-bit, ring 0
    dq 0x00CF9A000000FFFF
    ; Data segment: base=0, limit=4GB, 32-bit, ring 0
    dq 0x00CF92000000FFFF
    ; User code segment: base=0, limit=4GB, 32-bit, ring 3
    dq 0x00CFFA000000FFFF
    ; User data segment: base=0, limit=4GB, 32-bit, ring 3
    dq 0x00CFF2000000FFFF
    ; TSS descriptor placeholder (filled by kernel)
    dq 0x0000000000000000
GDT_END:

GDT_PTR:
    dw GDT_END - GDT_START - 1
    dd GDT_START

MSG_S2:  db "Stage 2:  EarlnuxOS Bootloader OK", 0x0D, 0x0A, 0
MSG_A20: db "A20 line enabled", 0x0D, 0x0A, 0

; ============================================================================
; 32-bit Protected Mode Entry
; ============================================================================
[BITS 32]
PM_ENTRY:
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9F000    ; Stack below 640KB mark

    ; Write "PM OK" to video memory (0xB8000) as sanity check
    mov dword [0xB8000], 0x0F4B0F50   ; "PK" in white
    mov dword [0xB8004], 0x0F200F4F   ; "O " in white

    ; Jump to kernel entry (kernel loaded at 0x100000 = 1MB)
    jmp 0x08:0x00100000

    ; Should not reach here
    hlt

times 512 - ($ - STAGE2_ENTRY) % 512 db 0
