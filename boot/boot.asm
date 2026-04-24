; ============================================================================
;  EarlnuxOS - Stage 1 Bootloader (MBR)
; Loads stage 2 from disk, sets up GDT, enters protected mode
; Author:  EarlnuxOS Team
; Target: x86, BIOS, 512 bytes
; ============================================================================

[BITS 16]
[ORG 0x7C00]

; ============================================================================
; BIOS loads us at 0x7C00. We fix segments, then load stage 2.
; ============================================================================

START:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7BFF

    ; Save drive number
    mov [BOOT_DRIVE], dl
    sti

    ; Print banner
    mov si, MSG_BOOT
    call PRINT_STR

    ; Reset disk
    xor ah, ah
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc DISK_ERROR

    ; Load Stage 2 (sectors 2-20) into 0x8000
    ; LBA mode via BIOS INT 13h AH=42h (Extended Read)
    mov si, DAP
    mov ah, 0x42
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc DISK_ERROR

    mov si, MSG_LOADED
    call PRINT_STR

    ; Far jump to Stage 2
    jmp 0x0000:0x8000

DISK_ERROR:
    mov si, MSG_ERR
    call PRINT_STR
    hlt

PRINT_STR:
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
; Disk Address Packet (DAP) for INT 13h Extended Read
; ============================================================================
DAP:
    db 0x10         ; Size of DAP
    db 0x00         ; Reserved
    dw 0x0040       ; Sectors to read (64 KB worth)
    dw 0x8000       ; Destination offset
    dw 0x0000       ; Destination segment
    dq 0x0001       ; Starting LBA (sector 1, zero-indexed)

BOOT_DRIVE:  db 0
MSG_BOOT:    db " EarlnuxOS v1.0 Booting...", 0x0D, 0x0A, 0
MSG_LOADED:  db "Stage 2 loaded. Entering kernel...", 0x0D, 0x0A, 0
MSG_ERR:     db "FATAL: Disk read error!", 0x0D, 0x0A, 0

; Pad to 510 bytes and add boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
