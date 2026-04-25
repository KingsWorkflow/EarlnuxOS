// Stub implementations for memory management functions

#include <mm/mm.h>
#include <kernel/kernel.h>

/* Static backing store — 512 KB bump allocator.
 * This guarantees kmalloc never returns NULL during early boot before
 * a real physical memory manager is wired up. */
#define STATIC_HEAP_SIZE (512 * 1024)
static char static_heap[STATIC_HEAP_SIZE];

static char   *heap_start = static_heap;
static size_t  heap_size  = STATIC_HEAP_SIZE;
static size_t  heap_used  = 0;

void *kmalloc(size_t size) {
    /* 8-byte align every allocation */
    size = (size + 7) & ~7u;
    if (heap_used + size > heap_size) return NULL;
    void *ptr = heap_start + heap_used;
    heap_used += size;
    return ptr;
}

void kfree(void *ptr) {
    (void)ptr; /* bump allocator — no free */
}

void pmm_init(mem_map_entry_t *entries, uint32_t count) {
    KINFO("MM", "PMM initialized with %u memory map entries", count);
    // Stub implementation
    (void)entries;
    (void)count;
}

void vmm_init(void) {
    KINFO("MM", "VMM initialized (stub)");
}

void heap_init(uintptr_t start, size_t size) {
    heap_start = (char *)start;
    heap_size = size;
    heap_used = 0;
    KINFO("MM", "Heap initialized at 0x%x, size %u KB", start, size / 1024);
}

void pmm_get_stats(pmm_stats_t *stats) {
    // Stub - fill with dummy values
    stats->total_phys = 64 * 1024 * 1024; // 64MB
    stats->free_phys = 32 * 1024 * 1024;  // 32MB
    stats->kernel_phys = 2 * 1024 * 1024; // 2MB
    stats->reserved_phys = 4 * 1024 * 1024; // 4MB
    stats->total_pages = stats->total_phys / 4096;
    stats->free_pages = stats->free_phys / 4096;
}

void heap_get_stats(heap_stats_t *stats) {
    // Fill with actual heap information
    stats->heap_start = (uint32_t)heap_start;
    stats->heap_end = (uint32_t)(heap_start + heap_size);
    stats->heap_size = heap_size;
    stats->used_bytes = heap_used;
    stats->free_bytes = heap_size - heap_used;
    stats->peak_usage = heap_used;
    stats->alloc_count = 1;
    stats->free_count = 0;
}

void pmm_dump_info(void) {
    KINFO("MM", "Physical Memory Manager (stub)");
}