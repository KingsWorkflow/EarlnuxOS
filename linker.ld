/* linker.ld - Linker script for kernel */

ENTRY(kernel_entry)

SECTIONS {
    . = 0x10000;  /* Load address (where bootloader puts kernel) */

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}