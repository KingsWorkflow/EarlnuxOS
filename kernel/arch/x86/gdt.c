/* ============================================================================
 *  EarlnuxOS - Global Descriptor Table (GDT) Implementation
 * kernel/arch/x86/gdt.c
 * ============================================================================ */

#include <arch/x86/gdt.h>
#include <kernel/kernel.h>

/* GDT entries - only need null, kernel code, kernel data for basic boot */
static gdt_entry_t gdt_entries[GDT_ENTRIES];

/* GDT pointer */
static gdt_ptr_t gdt_ptr;

/* Static function to set GDT entry */
static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = base & 0xFFFF;
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = limit & 0xFFFF;
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt_entries[num].access = access;
}

/* flush_gdt - defined in gdt_flush.asm, loads GDTR and reloads segments */
extern void gdt_flush(uint32_t gdt_ptr_addr);

/* Initialize GDT */
void gdt_init(void) {
    /* Null descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code segment: base=0, limit=4GB, DPL=0, 32-bit */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Kernel data segment: base=0, limit=4GB, DPL=0, 32-bit */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* User code segment: base=0, limit=4GB, DPL=3, 32-bit */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* User data segment: base=0, limit=4GB, DPL=3, 32-bit */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* TSS placeholder (null for now) */
    gdt_set_entry(5, 0, 0, 0, 0);

    /* Setup GDT pointer */
    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    /* Call assembly routine that does lgdt + far jump + reload segments safely */
    gdt_flush((uint32_t)&gdt_ptr);

    KINFO("GDT", "Initialized successfully\n");
}
