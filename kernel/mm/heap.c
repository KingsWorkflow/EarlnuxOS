/* ============================================================================
 *  EarlnuxOS - Kernel Heap (SLAB-based kmalloc)
 * mm/heap.c
 * ============================================================================ */

#include <mm/mm.h>
#include <kernel/kernel.h>

/* ============================================================================
 * Block header (placed before every allocated chunk)
 * ============================================================================ */
#define HEAP_MAGIC_FREE  0xDEADC0DEu
#define HEAP_MAGIC_USED  0xC0FFEE99u
#define HEAP_ALIGN       16

typedef struct block_hdr {
    uint32_t magic;
    size_t   size;       /* Payload size (not including header) */
    struct block_hdr *prev_phys; /* Previous physical block */
    struct block_hdr *next_free; /* Free list link */
    struct block_hdr *prev_free;
} block_hdr_t;

#define BLOCK_HDR_SIZE   ALIGN_UP(sizeof(block_hdr_t), HEAP_ALIGN)
#define MIN_BLOCK_SIZE   (BLOCK_HDR_SIZE + HEAP_ALIGN)

/* ============================================================================
 * Heap state
 * ============================================================================ */
static uint8_t  *heap_start_ptr = NULL;
static uint8_t  *heap_end_ptr   = NULL;
static uint8_t  *heap_brk       = NULL;

static block_hdr_t *free_list   = NULL;
static heap_stats_t heap_stat;

/* ============================================================================
 * Expand heap (called when no suitable free block found)
 * ============================================================================ */
static block_hdr_t *heap_expand(size_t min_size) {
    size_t expand = ALIGN_UP(min_size + BLOCK_HDR_SIZE, PAGE_SIZE);
    if (heap_brk + expand > heap_end_ptr) return NULL;

    block_hdr_t *blk = (block_hdr_t *)heap_brk;
    heap_brk += expand;
    blk->magic     = HEAP_MAGIC_FREE;
    blk->size      = expand - BLOCK_HDR_SIZE;
    blk->prev_phys = NULL;
    blk->next_free = free_list;
    blk->prev_free = NULL;
    if (free_list) free_list->prev_free = blk;
    free_list = blk;

    heap_stat.heap_end  = (size_t)heap_brk;
    heap_stat.free_bytes += blk->size;
    return blk;
}

/* ============================================================================
 * Remove a block from the free list
 * ============================================================================ */
static void free_list_remove(block_hdr_t *blk) {
    if (blk->prev_free) blk->prev_free->next_free = blk->next_free;
    else                free_list = blk->next_free;
    if (blk->next_free) blk->next_free->prev_free = blk->prev_free;
    blk->next_free = NULL;
    blk->prev_free = NULL;
}

/* ============================================================================
 * Split a free block if it's significantly larger than needed
 * ============================================================================ */
static void maybe_split(block_hdr_t *blk, size_t size) {
    if (blk->size < size + MIN_BLOCK_SIZE) return;

    /* Create a new free block for the remainder */
    block_hdr_t *remainder = (block_hdr_t *)((uint8_t *)blk + BLOCK_HDR_SIZE + size);
    remainder->magic     = HEAP_MAGIC_FREE;
    remainder->size      = blk->size - size - BLOCK_HDR_SIZE;
    remainder->prev_phys = blk;
    remainder->next_free = free_list;
    remainder->prev_free = NULL;
    if (free_list) free_list->prev_free = remainder;
    free_list = remainder;

    heap_stat.free_bytes += remainder->size;
    blk->size = size;
}

/* ============================================================================
 * Coalesce a freed block with adjacent free blocks
 * ============================================================================ */
static void coalesce(block_hdr_t *blk) {
    /* Coalesce with next physical block */
    block_hdr_t *next = (block_hdr_t *)((uint8_t *)blk + BLOCK_HDR_SIZE + blk->size);
    if ((uint8_t *)next < heap_brk && next->magic == HEAP_MAGIC_FREE) {
        free_list_remove(next);
        blk->size += BLOCK_HDR_SIZE + next->size;
        heap_stat.free_bytes += BLOCK_HDR_SIZE;
    }

    /* Coalesce with previous physical block */
    if (blk->prev_phys && blk->prev_phys->magic == HEAP_MAGIC_FREE) {
        block_hdr_t *prev = blk->prev_phys;
        free_list_remove(prev);
        prev->size += BLOCK_HDR_SIZE + blk->size;
        heap_stat.free_bytes += BLOCK_HDR_SIZE;
        /* Re-add prev to free list */
        prev->next_free = free_list;
        prev->prev_free = NULL;
        if (free_list) free_list->prev_free = prev;
        free_list = prev;
    } else {
        /* Add blk to free list */
        blk->next_free = free_list;
        blk->prev_free = NULL;
        if (free_list) free_list->prev_free = blk;
        free_list = blk;
    }
}

/* ============================================================================
 * Public heap API
 * ============================================================================ */
void heap_init(uint32_t virt_start, size_t size) {
    heap_start_ptr = (uint8_t *)virt_start;
    heap_end_ptr   = heap_start_ptr + size;
    heap_brk       = heap_start_ptr;

    heap_stat.heap_start  = virt_start;
    heap_stat.heap_end    = virt_start;
    heap_stat.heap_size   = size;
    heap_stat.used_bytes  = 0;
    heap_stat.free_bytes  = 0;
    heap_stat.alloc_count = 0;
    heap_stat.free_count  = 0;
    heap_stat.peak_usage  = 0;

    free_list = NULL;
    /* Pre-expand with one page */
    heap_expand(PAGE_SIZE);

    KINFO("heap", "Heap initialized at 0x%08X, capacity %u MB",
          virt_start, (uint32_t)(size / (1024 * 1024)));
}

void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = ALIGN_UP(size, HEAP_ALIGN);

    /* First-fit search */
    block_hdr_t *blk = free_list;
    while (blk) {
        if (blk->magic == HEAP_MAGIC_FREE && blk->size >= size) {
            free_list_remove(blk);
            maybe_split(blk, size);
            blk->magic = HEAP_MAGIC_USED;
            heap_stat.used_bytes += blk->size;
            heap_stat.free_bytes -= blk->size;
            heap_stat.alloc_count++;
            if (heap_stat.used_bytes > heap_stat.peak_usage)
                heap_stat.peak_usage = heap_stat.used_bytes;
            return (uint8_t *)blk + BLOCK_HDR_SIZE;
        }
        blk = blk->next_free;
    }

    /* Expand heap */
    blk = heap_expand(size);
    if (!blk) {
        KERROR("heap", "kmalloc(%u): out of memory!", (uint32_t)size);
        return NULL;
    }
    return kmalloc(size);  /* Retry */
}

void *kzalloc(size_t size) {
    void *p = kmalloc(size);
    if (p) {
        uint8_t *bp = (uint8_t *)p;
        for (size_t i = 0; i < size; i++) bp[i] = 0;
    }
    return p;
}

void *kmalloc_aligned(size_t size, size_t align) {
    /* Simple approach: allocate extra, align manually */
    if (align <= HEAP_ALIGN) return kmalloc(size);
    void *raw = kmalloc(size + align);
    if (!raw) return NULL;
    uintptr_t addr = (uintptr_t)raw;
    uintptr_t aligned = ALIGN_UP(addr, align);
    /* Note: we lose the raw pointer — this is a known limitation in bare-metal heaps.
     * Production would store the raw ptr at (aligned - sizeof(void*)) */
    return (void *)aligned;
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_hdr_t *blk = (block_hdr_t *)((uint8_t *)ptr - BLOCK_HDR_SIZE);

    if (blk->magic != HEAP_MAGIC_USED) {
        KERROR("heap", "kfree(%p): bad magic 0x%08X (double-free?)", ptr, blk->magic);
        return;
    }

    heap_stat.used_bytes -= blk->size;
    heap_stat.free_bytes += blk->size;
    heap_stat.free_count++;

    blk->magic = HEAP_MAGIC_FREE;
    coalesce(blk);
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr)      return kmalloc(new_size);
    if (!new_size) { kfree(ptr); return NULL; }

    block_hdr_t *blk = (block_hdr_t *)((uint8_t *)ptr - BLOCK_HDR_SIZE);
    if (blk->size >= new_size) return ptr;  /* Already large enough */

    void *np = kmalloc(new_size);
    if (!np) return NULL;

    /* Copy old content */
    uint8_t *src = (uint8_t *)ptr;
    uint8_t *dst = (uint8_t *)np;
    size_t copy_size = blk->size < new_size ? blk->size : new_size;
    for (size_t i = 0; i < copy_size; i++) dst[i] = src[i];

    kfree(ptr);
    return np;
}

size_t ksize(void *ptr) {
    if (!ptr) return 0;
    block_hdr_t *blk = (block_hdr_t *)((uint8_t *)ptr - BLOCK_HDR_SIZE);
    if (blk->magic != HEAP_MAGIC_USED) return 0;
    return blk->size;
}

void heap_get_stats(heap_stats_t *s) {
    *s = heap_stat;
}

void heap_dump_info(void) {
    KINFO("heap", "Heap: used=%u KB, free=%u KB, allocs=%u, frees=%u",
          (uint32_t)(heap_stat.used_bytes / 1024),
          (uint32_t)(heap_stat.free_bytes / 1024),
          (uint32_t)heap_stat.alloc_count,
          (uint32_t)heap_stat.free_count);
}
