/* ============================================================================
 *  EarlnuxOS - Global Descriptor Table (GDT) Implementation
 * kernel/arch/x86/gdt.c
 * ============================================================================ */

#include <arch/x86/gdt.h>
#include <arch/x86/ports.h>
#include <kernel/kernel.h>

/* GDT entries - 6 entries */
static gdt_entry_t gdt_entries[GDT_ENTRIES];

/* GDT pointer */
static gdt_ptr_t gdt_ptr;

/* Static function to set GDT entry */
static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags) {
    gdt_entries[num].base_low = base & 0xFFFF;
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = limit & 0xFFFF;
    gdt_entries[num].limit_high = (limit >> 16) & 0x0F;

    gdt_entries[num].access = access;
    gdt_entries[num].flags = flags & 0x0F;
}

/* Initialize GDT */
void gdt_init(void) {
    /* Null descriptor (required) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code segment: base=0, limit=4GB, type=0x9A (exec/read, DPL=0) */
    gdt_set_entry(KERNEL_CS, 0, 0xFFFFFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTABLE |
                  GDT_ACCESS_RW | GDT_ACCESS_DPL(0),
                  GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);

    /* Kernel data segment: base=0, limit=4GB, type=0x92 (read/write, DPL=0) */
    gdt_set_entry(KERNEL_DS, 0, 0xFFFFFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_DPL(0),
                  GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);

    /* User code segment: base=0, limit=4GB, type=0xFA (exec/read, DPL=3) */
    gdt_set_entry(USER_CS, 0, 0xFFFFFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTABLE |
                  GDT_ACCESS_RW | GDT_ACCESS_DPL(3),
                  GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);

    /* User data segment: base=0, limit=4GB, type=0xF2 (read/write, DPL=3) */
    gdt_set_entry(USER_DS, 0, 0xFFFFFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_DPL(3),
                  GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);

    /* TSS placeholder (not fully implemented yet) - will be set up later */
    gdt_set_entry(TSS_ENTRY, 0, 0,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_DPL(0),
                  0);

    /* Set up GDT pointer */
    gdt_ptr.limit = (uint16_t)(sizeof(gdt_entries) - 1);
    gdt_ptr.base = (uint32_t)&gdt_entries;

    /* Load GDT using assembly */
    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));

    /* Reload code and data segments */
    load_kernel_cs();
    load_kernel_ds();
}
