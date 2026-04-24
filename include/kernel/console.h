/* ============================================================================
 *  EarlnuxOS - VGA Text-Mode Console API
 * include/kernel/console.h
 * ============================================================================ */

#ifndef  EarlnuxOS_CONSOLE_H
#define  EarlnuxOS_CONSOLE_H

#include <types.h>

/* VGA text buffer base address */
#define VGA_BASE    0xB8000
#define VGA_COLS    80
#define VGA_ROWS    25

/* VGA color palette */
typedef enum {
    COLOR_BLACK         = 0,
    COLOR_BLUE          = 1,
    COLOR_GREEN         = 2,
    COLOR_CYAN          = 3,
    COLOR_RED           = 4,
    COLOR_MAGENTA       = 5,
    COLOR_BROWN         = 6,
    COLOR_LIGHT_GRAY    = 7,
    COLOR_DARK_GRAY     = 8,
    COLOR_LIGHT_BLUE    = 9,
    COLOR_LIGHT_GREEN   = 10,
    COLOR_LIGHT_CYAN    = 11,
    COLOR_LIGHT_RED     = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW        = 14,
    COLOR_WHITE         = 15,
} vga_color_t;

/* Color attribute: foreground << 4 | background */
#define VGA_ATTR(fg, bg)   ((uint8_t)(((bg) << 4) | ((fg) & 0x0F)))
#define VGA_DEFAULT_ATTR   VGA_ATTR(COLOR_LIGHT_GRAY, COLOR_BLACK)
#define VGA_KERNEL_ATTR    VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK)
#define VGA_ERROR_ATTR     VGA_ATTR(COLOR_WHITE,      COLOR_RED)
#define VGA_SUCCESS_ATTR   VGA_ATTR(COLOR_LIGHT_GREEN,COLOR_BLACK)
#define VGA_WARN_ATTR      VGA_ATTR(COLOR_YELLOW,     COLOR_BLACK)

/* Console API */
void console_init(void);
void console_clear(void);
void console_putchar(char c);
void console_putchar_color(char c, uint8_t attr);
void console_puts(const char *s);
void console_puts_color(const char *s, uint8_t attr);
void console_set_color(uint8_t attr);
void console_set_cursor(uint8_t row, uint8_t col);
void console_get_cursor(uint8_t *row, uint8_t *col);

/* Write a formatted string to console (like printf) */
int  console_printf(const char *fmt, ...);

#endif /*  EarlnuxOS_CONSOLE_H */
