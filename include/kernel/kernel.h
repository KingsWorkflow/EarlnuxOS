/* ============================================================================
 *  EarlnuxOS - Kernel Core Header
 * kernel/kernel.h
 * ============================================================================ */

#ifndef  EarlnuxOS_KERNEL_H
#define  EarlnuxOS_KERNEL_H

#include <types.h>

/* ============================================================================
 * Version information
 * ============================================================================ */
#define  EarlnuxOS_NAME          " EarlnuxOS"
#define  EarlnuxOS_VERSION_MAJOR 1
#define  EarlnuxOS_VERSION_MINOR 0
#define  EarlnuxOS_VERSION_PATCH 0
#define  EarlnuxOS_VERSION_STR   "1.0.0"
#define  EarlnuxOS_CODENAME      "Prism"

/* ============================================================================
 * VGA Text-Mode Console
 * ============================================================================ */
#define VGA_BASE        0xB8000
#define VGA_COLS        80
#define VGA_ROWS        25

/* VGA colors */
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

#define VGA_ATTR(fg, bg)   ((uint8_t)(((bg) << 4) | ((fg) & 0x0F)))
#define VGA_DEFAULT_ATTR   VGA_ATTR(COLOR_LIGHT_GRAY, COLOR_BLACK)
#define VGA_KERNEL_ATTR    VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK)
#define VGA_ERROR_ATTR     VGA_ATTR(COLOR_WHITE,      COLOR_RED)
#define VGA_SUCCESS_ATTR   VGA_ATTR(COLOR_LIGHT_GREEN,COLOR_BLACK)
#define VGA_WARN_ATTR      VGA_ATTR(COLOR_YELLOW,     COLOR_BLACK)

/* Console functions */
void console_init(void);
void console_clear(void);
void console_putchar(char c);
void console_putchar_color(char c, uint8_t attr);
void console_puts(const char *s);
void console_puts_color(const char *s, uint8_t attr);
void console_set_color(uint8_t attr);
void console_set_cursor(uint8_t row, uint8_t col);
void console_get_cursor(uint8_t *row, uint8_t *col);

/* ============================================================================
 * Printf / Kernel Logging
 * ============================================================================ */
int  kprintf(const char *fmt, ...);
int  ksprintf(char *buf, const char *fmt, ...);
int  ksnprintf(char *buf, size_t n, const char *fmt, ...);

/* Log levels */
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARN    2
#define LOG_ERROR   3
#define LOG_FATAL   4

void klog(int level, const char *module, const char *fmt, ...);
#define KDEBUG(mod, fmt, ...) klog(LOG_DEBUG, mod, fmt, ##__VA_ARGS__)
#define KINFO(mod, fmt, ...)  klog(LOG_INFO,  mod, fmt, ##__VA_ARGS__)
#define KWARN(mod, fmt, ...)  klog(LOG_WARN,  mod, fmt, ##__VA_ARGS__)
#define KERROR(mod, fmt, ...) klog(LOG_ERROR, mod, fmt, ##__VA_ARGS__)

/* ============================================================================
 * Panic / Assertions
 * ============================================================================ */
void panic_at(const char *file, int line, const char *msg) NORETURN;
void panic(const char *fmt, ...) NORETURN;

#define ASSERT(cond) \
    do { if (UNLIKELY(!(cond))) \
        panic_at(__FILE__, __LINE__, "Assertion failed: " #cond); \
    } while (0)

#define ASSERT_MSG(cond, msg) \
    do { if (UNLIKELY(!(cond))) \
        panic_at(__FILE__, __LINE__, msg); \
    } while (0)

/* ============================================================================
 * Port I/O
 * ============================================================================ */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port) : "memory");
}
static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port) : "memory");
    return v;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t v;
    __asm__ volatile("inw %1, %0" : "=a"(v) : "Nd"(port) : "memory");
    return v;
}
static inline uint32_t inl(uint16_t port) {
    uint32_t v;
    __asm__ volatile("inl %1, %0" : "=a"(v) : "Nd"(port) : "memory");
    return v;
}

/* I/O delay (burn ~1 microsecond) */
static inline void io_wait(void) { outb(0x80, 0); }

/* Memory barrier */
static inline void mb(void) {
    __asm__ volatile("" : : : "memory");
}

/* CPU HLT */
static inline void cpu_hlt(void) {
    __asm__ volatile("hlt");
}

/* Enable/disable interrupts */
static inline void sti(void) { __asm__ volatile("sti" : : : "memory"); }
static inline void cli(void) { __asm__ volatile("cli" : : : "memory"); }

/* Read CR registers */
static inline uint32_t read_cr0(void) {
    uint32_t v; __asm__ volatile("mov %%cr0, %0" : "=r"(v)); return v;
}
static inline uint32_t read_cr2(void) {
    uint32_t v; __asm__ volatile("mov %%cr2, %0" : "=r"(v)); return v;
}
static inline uint32_t read_cr3(void) {
    uint32_t v; __asm__ volatile("mov %%cr3, %0" : "=r"(v)); return v;
}

/* ============================================================================
 * Kernel init entry point
 * ============================================================================ */
void kernel_main(void) NORETURN;
void kernel_early_init(void);

#endif /*  EarlnuxOS_KERNEL_H */
