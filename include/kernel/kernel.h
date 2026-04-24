/* ============================================================================
 *  EarlnuxOS - Kernel Core Header
 * kernel/kernel.h
 * ============================================================================ */

#ifndef  EarlnuxOS_KERNEL_H
#define  EarlnuxOS_KERNEL_H

#include <types.h>
#include <kernel/console.h>

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
 * Multiboot structures
 * ============================================================================ */
typedef struct PACKED multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;     /* Kilobytes below 1MB */
    uint32_t mem_upper;     /* Kilobytes above 1MB */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
} multiboot_info_t;

typedef struct PACKED mmap_entry {
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} mmap_entry_t;

#define MB_FLAG_MMAP        BIT(6)
#define MULTIBOOT_MAGIC_VAL 0x2BADB002u

/* ============================================================================
 * Kernel init entry point
 * ============================================================================ */
void kernel_main(uint32_t mb_magic, multiboot_info_t *mb_info) NORETURN;
void kernel_early_init(void);

#endif /*  EarlnuxOS_KERNEL_H */
