/* ============================================================================
 *  EarlnuxOS - VGA Text-Mode Console Driver
 * kernel/console.c
 * ============================================================================ */

#include <kernel/console.h>
#include <kernel/kernel.h>
#include <types.h>
#include <arch/x86/ports.h>

/* VGA text buffer location */
static uint16_t *vga_buffer = (uint16_t *)VGA_BASE;
static uint8_t vga_color = VGA_DEFAULT_ATTR;
static uint8_t console_row = 0;
static uint8_t console_col = 0;

/* Forward declarations */
static void console_scroll(void);
static void console_put_cursor(void);

/* Initialize the VGA text console */
void console_init(void) {
    console_row = 0;
    console_col = 0;
    vga_color = VGA_DEFAULT_ATTR;
    console_clear();
}

/* Clear the screen */
void console_clear(void) {
    for (uint8_t row = 0; row < VGA_ROWS; row++) {
        for (uint8_t col = 0; col < VGA_COLS; col++) {
            vga_buffer[row * VGA_COLS + col] = (uint16_t)' ' | (uint16_t)vga_color << 8;
        }
    }
    console_row = 0;
    console_col = 0;
    console_put_cursor();
}

/* Put a single character (with color) at current position */
static void console_put_entry_at(char c, uint8_t color, uint8_t row, uint8_t col) {
    vga_buffer[row * VGA_COLS + col] = (uint16_t)c | (uint16_t)color << 8;
}

/* Put a character with default color */
void console_putchar(char c) {
    console_putchar_color(c, vga_color);
}

/* Put a character with specific color */
void console_putchar_color(char c, uint8_t attr) {
    if (c == '\n') {
        console_col = 0;
        console_row++;
    } else if (c == '\r') {
        console_col = 0;
    } else if (c == '\t') {
        console_col = (console_col + 8) & ~7;
    } else if (c == '\b') {
        if (console_col > 0) {
            console_col--;
            console_put_entry_at(' ', attr, console_row, console_col);
        }
    } else {
        console_put_entry_at(c, attr, console_row, console_col);
        console_col++;
        if (console_col >= VGA_COLS) {
            console_col = 0;
            console_row++;
        }
    }

    if (console_row >= VGA_ROWS) {
        console_scroll();
    }
    console_put_cursor();
}

/* Put a string */
void console_puts(const char *s) {
    while (*s) {
        console_putchar(*s++);
    }
}

/* Put a string with color */
void console_puts_color(const char *s, uint8_t attr) {
    uint8_t old_color = vga_color;
    vga_color = attr;
    while (*s) {
        console_putchar(*s++);
    }
    vga_color = old_color;
}

/* Set console text color */
void console_set_color(uint8_t attr) {
    vga_color = attr;
}

/* Set cursor position */
void console_set_cursor(uint8_t row, uint8_t col) {
    if (row < VGA_ROWS && col < VGA_COLS) {
        console_row = row;
        console_col = col;
        console_put_cursor();
    }
}

/* Get current cursor position */
void console_get_cursor(uint8_t *row, uint8_t *col) {
    if (row) *row = console_row;
    if (col) *col = console_col;
}

/* Scroll screen up one line */
static void console_scroll(void) {
    /* Move all lines up one */
    for (uint8_t row = 1; row < VGA_ROWS; row++) {
        for (uint8_t col = 0; col < VGA_COLS; col++) {
            vga_buffer[(row - 1) * VGA_COLS + col] = vga_buffer[row * VGA_COLS + col];
        }
    }

    /* Clear bottom line */
    for (uint8_t col = 0; col < VGA_COLS; col++) {
        vga_buffer[(VGA_ROWS - 1) * VGA_COLS + col] = (uint16_t)' ' | (uint16_t)vga_color << 8;
    }

    console_row = VGA_ROWS - 1;
    console_put_cursor();
}

/* Update hardware cursor position */
static void console_put_cursor(void) {
    uint16_t pos = console_row * VGA_COLS + console_col;

    /* Cursor low port */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    /* Cursor high port */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
