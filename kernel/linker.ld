/* ============================================================================
 *  EarlnuxOS Kernel Linker Script
 * kernel/linker.ld
 * ============================================================================ */

/* Output format: 32-bit ELF */
OUTPUT_FORMAT(elf32-i386)

/* Entry point: defined in entry.asm */
ENTRY(_start)

/* Kernel virtual base address (higher-half) */
#define KERNEL_VIRT_BASE  0xC0000000

/* Physical load address (where kernel is loaded by bootloader) */
#define KERNEL_PHYS_BASE  0x00100000

/* Start of kernel virtual addresses */
#define KERNEL_START      (KERNEL_VIRT_BASE + KERNEL_PHYS_BASE)

SECTIONS
{
    /* ====================================================================== */
    /* Physical addresses (bootloader loads at KERNEL_PHYS_BASE)            */
    /* ====================================================================== */

    . = KERNEL_START;

    .text : AT(ADDR(.text) - KERNEL_VIRT_BASE) {
        *(.multiboot)          /* Multiboot header (first for alignment) */
        *(.text)               /* Code sections */
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    }

    .data : AT(ADDR(.data) - KERNEL_VIRT_BASE) {
        *(.data)
        *(.data.*)
    }

    .bss : AT(ADDR(.bss) - KERNEL_VIRT_BASE) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    /* Heap region (kernel dynamic memory) - placed after BSS */
    . = ALIGN(0x1000);
    __kernel_heap_start = .;
    __kernel_heap_end = . + 0x10000000; /* 256 MB heap */

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}

/* Provide symbols for use in C code */
PROVIDE(KERNEL_VIRT_BASE = 0xC0000000);
PROVIDE(KERNEL_PHYS_BASE = 0x00100000);
PROVIDE(KERNEL_START = .); /* Not needed? Already computed */
