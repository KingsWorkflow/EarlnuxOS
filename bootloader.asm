; bootloader.asm - Real mode bootloader (16-bit)
; Assembled with: nasm -f bin bootloader.asm -o bootloader.bin

[ORG 0x7C00]
[BITS 16]

START:
    ; Clear screen
    mov ax, 0x0600          ; AH=06 (scroll), AL=00 (clear)
    mov bh, 0x07            ; Light gray on black
    mov cx, 0               ; Top-left
    mov dx, 0x184F          ; Bottom-right (24, 79)
    int 0x10

    ; Set cursor to (0, 0)
    mov ah, 0x02
    xor bh, bh              ; Page 0
    xor dx, dx              ; Row 0, Col 0
    int 0x10

    ; Print "OS Loading..."
    mov si, BOOT_MSG
    call PRINT_STRING

    ; Load kernel into memory
    ; Bootloader is at sector 0
    ; Kernel starts at sector 10 (0x0A)
    
    mov ah, 0x02            ; Read sectors
    mov al, 10              ; Read 10 sectors (5KB)
    mov ch, 0               ; Cylinder 0
    mov cl, 11              ; Sector 11 (sector 10 is 0-indexed as sector 11)
    mov dh, 0               ; Head 0
    mov dl, 0               ; Drive 0 (floppy)
    mov bx, 0x1000          ; Load into 0x1000:0x0000 = 0x10000
    mov es, bx
    xor bx, bx
    int 0x13

    ; Check for error
    jc LOAD_ERROR

    ; Far jump to kernel
    ; Kernel assumes it's loaded at 0x10000
    jmp 0x1000:0x0000

LOAD_ERROR:
    mov si, ERROR_MSG
    call PRINT_STRING
    hlt

PRINT_STRING:
    ; Input: SI = pointer to null-terminated string
    ; Uses BIOS interrupt 0x10
    mov ah, 0x0E            ; Teletype output
    xor bh, bh              ; Page 0
.loop:
    lodsb                   ; Load byte from DS:SI into AL, increment SI
    test al, al             ; Check for null terminator
    jz .done
    int 0x10                ; Print character
    jmp .loop
.done:
    ret

BOOT_MSG: db "OS Loading...", 0
ERROR_MSG: db "Load Error!", 0

; Padding to 512 bytes - 2 (for boot signature)
times 510 - ($ - $$) db 0
dw 0xAA55                   ; Boot signature