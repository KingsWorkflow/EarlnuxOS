/* ============================================================================
 *  EarlnuxOS - Memory Management Subsystem Header
 * include/mm/mm.h
 * ============================================================================ */

#ifndef  EarlnuxOS_MM_H
#define  EarlnuxOS_MM_H

#include <types.h>

/* ============================================================================
 * Physical Memory Manager (PMM) - Buddy Allocator
 * ============================================================================ */
#define PMM_MAX_ORDER       11          /* 2^11 pages = 8MB max contiguous block */
#define PMM_ZONE_DMA        0           /* 0-16MB */
#define PMM_ZONE_NORMAL     1           /* 16MB-896MB */
#define PMM_ZONE_HIGHMEM    2           /* >896MB */
#define PMM_ZONE_COUNT      3

/* Physical memory map entry types (from BIOS/multiboot) */
typedef enum {
    MEMTYPE_USABLE      = 1,
    MEMTYPE_RESERVED    = 2,
    MEMTYPE_ACPI_RECLAIM= 3,
    MEMTYPE_ACPI_NVS    = 4,
    MEMTYPE_BAD         = 5,
} mem_type_t;

typedef struct mem_map_entry {
    uint64_t base;
    uint64_t length;
    mem_type_t type;
} PACKED mem_map_entry_t;

/* Page frame descriptor */
typedef struct page {
    uint32_t flags;         /* Page state flags */
    uint32_t ref_count;     /* Reference count */
    uint32_t order;         /* Buddy order if free */
    struct page *next;      /* Free list linkage */
    struct page *prev;
    void *virt_addr;        /* Mapped virtual address (if any) */
} page_t;

/* Page flags */
#define PAGE_FLAG_FREE      BIT(0)
#define PAGE_FLAG_RESERVED  BIT(1)
#define PAGE_FLAG_KERNEL    BIT(2)
#define PAGE_FLAG_USER      BIT(3)
#define PAGE_FLAG_DMA       BIT(4)
#define PAGE_FLAG_DIRTY     BIT(5)
#define PAGE_FLAG_ACCESSED  BIT(6)
#define PAGE_FLAG_LOCKED    BIT(7)

/* Buddy system free list (one per order per zone) */
typedef struct buddy_zone {
    struct page *free_list[PMM_MAX_ORDER + 1];
    size_t free_count[PMM_MAX_ORDER + 1];
    uint32_t base_pfn;
    uint32_t end_pfn;
    size_t total_pages;
    size_t free_pages;
    const char *name;
} buddy_zone_t;

/* PMM statistics */
typedef struct pmm_stats {
    size_t total_phys;
    size_t usable_phys;
    size_t free_phys;
    size_t kernel_phys;
    size_t reserved_phys;
    uint32_t total_pages;
    uint32_t free_pages;
} pmm_stats_t;

/* PMM API */
void     pmm_init(mem_map_entry_t *map, uint32_t count);
uint32_t pmm_alloc_page(void);
uint32_t pmm_alloc_pages(uint32_t order);
void     pmm_free_page(uint32_t phys);
void     pmm_free_pages(uint32_t phys, uint32_t order);
void     pmm_get_stats(pmm_stats_t *stats);
void     pmm_dump_info(void);

/* Conversion helpers */
static inline uint32_t phys_to_pfn(uint32_t phys) { return phys >> PAGE_SHIFT; }
static inline uint32_t pfn_to_phys(uint32_t pfn)  { return pfn << PAGE_SHIFT; }

/* ============================================================================
 * Virtual Memory Manager (VMM) - Paging (x86 Two-Level)
 * ============================================================================ */

/* Page directory / table entry flags */
#define PTE_PRESENT     BIT(0)
#define PTE_WRITE       BIT(1)
#define PTE_USER        BIT(2)
#define PTE_WRITETHROUGH BIT(3)
#define PTE_NOCACHE     BIT(4)
#define PTE_ACCESSED    BIT(5)
#define PTE_DIRTY       BIT(6)
#define PTE_HUGE        BIT(7)   /* 4MB pages in PDE */
#define PTE_GLOBAL      BIT(8)
#define PTE_ADDR_MASK   0xFFFFF000u

#define PDE_INDEX(virt)  (((virt) >> 22) & 0x3FF)
#define PTE_INDEX(virt)  (((virt) >> 12) & 0x3FF)

typedef uint32_t pde_t;
typedef uint32_t pte_t;
typedef pde_t    pgdir_t[1024];
typedef pte_t    pgtable_t[1024];

/* VMM API */
void     vmm_init(void);
int      vmm_map_page(pgdir_t *dir, uint32_t virt, uint32_t phys, uint32_t flags);
void     vmm_unmap_page(pgdir_t *dir, uint32_t virt);
uint32_t vmm_virt_to_phys(pgdir_t *dir, uint32_t virt);
pgdir_t *vmm_clone_dir(pgdir_t *src);
void     vmm_destroy_dir(pgdir_t *dir);
void     vmm_switch_dir(pgdir_t *dir);
void     vmm_flush_tlb(uint32_t virt);
void     vmm_flush_tlb_all(void);

/* Kernel page directory (identity + higher-half) */
extern pgdir_t *kernel_pgdir;

/* Identity map a range */
void vmm_identity_map(uint32_t start, uint32_t end, uint32_t flags);

/* Map physical range to virtual range */
void vmm_map_range(pgdir_t *dir, uint32_t virt, uint32_t phys,
                   size_t size, uint32_t flags);

/* ============================================================================
 * Kernel Heap Allocator (SLAB-based)
 * ============================================================================ */

/* Slab cache: fixed-size object pool */
typedef struct kmem_cache {
    const char *name;
    size_t      obj_size;       /* Object size in bytes */
    size_t      obj_align;      /* Alignment requirement */
    uint32_t    objs_per_slab;  /* Objects per slab page */
    /* Slab lists */
    void       *partial;        /* Partially used slabs */
    void       *full;           /* Full slabs */
    void       *empty;          /* Empty slabs */
    /* Stats */
    uint32_t    total_allocs;
    uint32_t    total_frees;
    uint32_t    current_allocs;
    struct kmem_cache *next;    /* Global cache list */
} kmem_cache_t;

/* Core kmalloc size classes (bytes) */
#define KMALLOC_MIN     8
#define KMALLOC_MAX     4096

/* Heap API */
void  heap_init(uint32_t virt_start, size_t size);
void *kmalloc(size_t size);
void *kmalloc_aligned(size_t size, size_t align);
void *kzalloc(size_t size);
void *krealloc(void *ptr, size_t new_size);
void  kfree(void *ptr);
size_t ksize(void *ptr);

/* Slab cache API */
kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align);
void         *kmem_cache_alloc(kmem_cache_t *cache);
void          kmem_cache_free(kmem_cache_t *cache, void *obj);
void          kmem_cache_destroy(kmem_cache_t *cache);

/* Heap stats */
typedef struct heap_stats {
    size_t heap_start;
    size_t heap_end;
    size_t heap_size;
    size_t used_bytes;
    size_t free_bytes;
    size_t alloc_count;
    size_t free_count;
    size_t peak_usage;
} heap_stats_t;

void heap_get_stats(heap_stats_t *stats);
void heap_dump_info(void);

/* ============================================================================
 * Virtual Memory Area (VMA) - process address space regions
 * ============================================================================ */
#define VMA_READ    BIT(0)
#define VMA_WRITE   BIT(1)
#define VMA_EXEC    BIT(2)
#define VMA_SHARED  BIT(3)
#define VMA_ANON    BIT(4)
#define VMA_STACK   BIT(5)
#define VMA_HEAP    BIT(6)

typedef struct vma {
    uint32_t    vm_start;
    uint32_t    vm_end;
    uint32_t    vm_flags;
    const char *vm_name;
    struct vma *vm_next;
} vma_t;

/* VMA API */
vma_t *vma_create(uint32_t start, uint32_t end, uint32_t flags, const char *name);
void   vma_destroy(vma_t *vma);
vma_t *vma_find(vma_t *list, uint32_t addr);
int    vma_insert(vma_t **list, vma_t *vma);

/* ============================================================================
 * Kernel memory layout constants
 * ============================================================================ */
#define KERNEL_LOAD_PHYS    0x00100000u  /* Kernel loaded at 1MB physical */
#define KERNEL_VIRT_BASE    0xC0000000u  /* Higher-half: kernel virtual base */
#define KERNEL_HEAP_START   0xD0000000u  /* Kernel heap */
#define KERNEL_HEAP_SIZE    (256 * 1024 * 1024u)  /* 256 MB heap */
#define KERNEL_STACK_TOP    0xCFFFFFF0u
#define KERNEL_STACK_SIZE   (64 * 1024u)           /* 64 KB stack */
#define USER_SPACE_START    0x00001000u
#define USER_SPACE_END      0xBFFFFFFFu
#define MMIO_BASE           0xF0000000u

/* Memory-mapped I/O helper */
static inline uint32_t mmio_read32(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}
static inline void mmio_write32(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

#endif /*  EarlnuxOS_MM_H */
