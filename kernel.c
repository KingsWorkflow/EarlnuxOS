// kernel.c - Main kernel code
// Compiled with: i686-elf-gcc -ffreestanding -O2 -c kernel.c -o kernel.o

#include <stdint.h>

extern void idt_init(void);

/* Video memory starts at 0xB8000 */
#define VIDEO_MEMORY (uint8_t *)0xB8000
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

/* Cursor position */
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

/* Print a single character */
void print_char(char c, uint8_t color) {
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= SCREEN_HEIGHT) {
        // Scroll screen (simplified: just wrap to top)
        cursor_y = 0;
    }

    uint16_t offset = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
    VIDEO_MEMORY[offset]     = c;      // Character
    VIDEO_MEMORY[offset + 1] = color;  // Attribute (light gray on black = 0x07)

    cursor_x++;
}

/* Print null-terminated string */
void print_string(const char *str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i], color);
    }
}

/* Handle newline */
void print_newline(void) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        cursor_y = 0;
    }
}

/* Main kernel function */
void main(void) {
    print_string("Hello from kernel!", 0x0F);  // 0x0F = white on black
    print_newline();
    print_string("Press any key...", 0x07);    // 0x07 = light gray on black

    idt_init();

    /* Infinite loop (CPU will be interrupted by keyboard) */
    while (1) {
        asm("hlt");  // Halt until interrupt
    }
}