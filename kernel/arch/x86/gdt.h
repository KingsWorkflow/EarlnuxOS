/* ============================================================================
 *  EarlnuxOS - Global Descriptor Table (GDT)
 * kernel/arch/x86/gdt.h
 * ============================================================================ */

#ifndef  EarlnuxOS_ARCH_X86_GDT_H
#define  EarlnuxOS_ARCH_X86_GDT_H

#include <types.h>

/* GDT entry count */
#define GDT_ENTRIES    6

/* GDT segment selector offsets */
#define KERNEL_CS      1
#define KERNEL_DS      2
#define USER_CS        3
#define USER_DS        4
#define TSS_ENTRY      5

/* Segment descriptor structure (8 bytes) */
typedef struct PACKED {
    uint16_t limit_low;       /* Low 16 bits of segment limit */
    uint16_t base_low;        /* Low 16 bits of base address */
    uint8_t  base_middle;     /* Middle 8 bits of base */
    uint8_t  access;          /* Access flags */
    uint8_t  limit_high:4;    /* High 4 bits of limit */
    uint8_t  flags:4;         /* Flags */
    uint8_t  base_high;       /* High 8 bits of base */
} gdt_entry_t;

/* GDT pointer structure */
typedef struct PACKED {
    uint16_t limit;           /* Size of GDT */
    uint32_t base;            /* Address of GDT */
} gdt_ptr_t;

/* Access byte flags */
#define GDT_ACCESS_PRESENT    (1 << 7)   /* Segment present in memory */
#define GDT_ACCESS_DPL(x)     (((x) & 0x3) << 5) /* Descriptor privilege level */
#define GDT_ACCESS_SEGMENT    (1 << 4)   /* Segment descriptor (vs system) */
#define GDT_ACCESS_EXECUTABLE (1 << 3)   /* Executable segment */
#define GDT_ACCESS_DIRECTION  (1 << 2)   /* Direction (0=up, 1=down) */
#define GDT_ACCESS_RW         (1 << 1)   /* Read/write permission */
#define GDT_ACCESS_ACCESSED   (1 << 0)   /* CPU set when accessed */

/* Flags (high 4 bits of limit_high + flags) */
#define GDT_FLAG_GRANULARITY  (1 << 3)   /* Granularity: 1=4KB, 0=1 byte */
#define GDT_FLAG_SIZE         (1 << 2)   /* Size: 1=32-bit, 0=16-bit */
#define GDT_FLAG_LONG_MODE    (1 << 1)   /* Long mode (64-bit) */

/* Convenience macros for segment type */
#define GDT_ENTRY_KERNEL_CODE 0x00CF9A000000FFFF  /* Kernel code segment */
#define GDT_ENTRY_KERNEL_DATA 0x00CF92000000FFFF  /* Kernel data segment */
#define GDT_ENTRY_USER_CODE   0x00CFFA000000FFFF  /* User code segment */
#define GDT_ENTRY_USER_DATA   0x00CFF2000000FFFF  /* User data segment */
#define GDT_ENTRY_TSS         0x0089E80000000000  /* TSS (early placeholder) */

/* Initialize GDT */
void gdt_init(void);

/* Load segment registers */
static inline void load_kernel_cs(void) {
    __asm__ volatile("ljmp %0, $1f\n1:" : : "i"(KERNEL_CS << 3));
}

static inline void load_kernel_ds(void) {
    __asm__ volatile("movw %0, %%ds\n"
                     "movw %0, %%es\n"
                     "movw %0, %%fs\n"
                     "movw %0, %%gs\n"
                     "movw %0, %%ss"
                     : : "i"(KERNEL_DS << 3) : "memory");
}

/* Reload data segments after changing privilege level */
static inline void reload_segments(void) {
    __asm__ volatile("movw %0, %%ds\n"
                     "movw %0, %%es\n"
                     "movw %0, %%fs\n"
                     "movw %0, %%gs\n"
                     "movw %0, %%ss"
                     : : "i"(KERNEL_DS << 3) : "memory");
}

#endif /*  EarlnuxOS_ARCH_X86_GDT_H */
