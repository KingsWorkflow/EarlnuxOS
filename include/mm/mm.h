#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include <stdint.h>
#include <stddef.h>

// Memory map entry types
typedef enum {
    MEMTYPE_USABLE = 1,
    MEMTYPE_RESERVED = 2,
    MEMTYPE_ACPI_RECLAIM = 3,
    MEMTYPE_ACPI_NVS = 4,
    MEMTYPE_BAD = 5
} mem_type_t;

typedef struct {
    uint64_t base;
    uint64_t length;
    mem_type_t type;
} mem_map_entry_t;

// PMM statistics
typedef struct {
    uint32_t total_phys;
    uint32_t free_phys;
    uint32_t kernel_phys;
    uint32_t reserved_phys;
    uint32_t total_pages;
    uint32_t free_pages;
} pmm_stats_t;

// Heap statistics
typedef struct {
    uint32_t heap_start;
    uint32_t heap_end;
    uint32_t heap_size;
    uint32_t used_bytes;
    uint32_t free_bytes;
    uint32_t peak_usage;
    uint32_t alloc_count;
    uint32_t free_count;
} heap_stats_t;

// Memory management functions
void *kmalloc(size_t size);
void kfree(void *ptr);

// Memory initialization
void pmm_init(mem_map_entry_t *entries, uint32_t count);
void vmm_init(void);
void heap_init(uintptr_t start, size_t size);

// Statistics functions
void pmm_get_stats(pmm_stats_t *stats);
void heap_get_stats(heap_stats_t *stats);
void pmm_dump_info(void);

// Memory type constants (for compatibility)
#define MEMTYPE_USABLE MEMTYPE_USABLE
#define MEMTYPE_RESERVED MEMTYPE_RESERVED
#define MEMTYPE_ACPI_RECLAIM MEMTYPE_ACPI_RECLAIM
#define MEMTYPE_ACPI_NVS MEMTYPE_ACPI_NVS
#define MEMTYPE_BAD MEMTYPE_BAD

#endif