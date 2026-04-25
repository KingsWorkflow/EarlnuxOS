/* linker.ld - EarlnuxOS kernel linker script */

ENTRY(_start)

SECTIONS {
    /* Standard Multiboot2 load address at 1MB */
    . = 1M;

    /* Kernel entry and Multiboot header MUST be first */
    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }
}