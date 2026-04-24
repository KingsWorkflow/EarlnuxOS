/* ============================================================================
 *  EarlnuxOS - Core Type Definitions
 * include/kernel/types.h
 * ============================================================================ */

#ifndef  EarlnuxOS_TYPES_H
#define  EarlnuxOS_TYPES_H

/* Primitive fixed-width types */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;
typedef uint32_t            pid_t;
typedef uint32_t            uid_t;
typedef uint64_t            uint64_time_t;

/* Boolean */
typedef int                 bool;
#define true   1
#define false  0
#define TRUE   1
#define FALSE  0

/* NULL */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* Useful macros */
#define ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define CLAMP(v, lo, hi) (MIN(MAX(v, lo), hi))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define OFFSET_OF(type, member) ((size_t)&((type *)0)->member)
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - OFFSET_OF(type, member)))

/* Bit operations */
#define BIT(n)          (1u << (n))
#define BIT_SET(x, n)   ((x) |= BIT(n))
#define BIT_CLR(x, n)   ((x) &= ~BIT(n))
#define BIT_TEST(x, n)  (!!((x) & BIT(n)))
#define BIT_FLIP(x, n)  ((x) ^= BIT(n))

/* Page size */
#define PAGE_SIZE       4096u
#define PAGE_SHIFT      12
#define PAGE_MASK       (~(PAGE_SIZE - 1))

/* Compiler hints */
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define NORETURN    __attribute__((noreturn))
#define PACKED      __attribute__((packed))
#define ALIGNED(n)  __attribute__((aligned(n)))
#define UNUSED      __attribute__((unused))
#define NOINLINE    __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

/* Panic macro (defined in kernel.h, declared here) */
#define PANIC(msg) panic_at(__FILE__, __LINE__, msg)

/* Kernel virtual/physical address macros */
#define KERNEL_BASE     0xC0000000u
#define VIRT_TO_PHYS(x) ((uint32_t)(x) - KERNEL_BASE)
#define PHYS_TO_VIRT(x) ((void *)((uint32_t)(x) + KERNEL_BASE))

/* Kernel heap region */
#define KERNEL_HEAP_START  0xD0000000u
#define KERNEL_HEAP_SIZE   (256 * 1024 * 1024u)  /* 256 MB */

#endif /*  EarlnuxOS_TYPES_H */
