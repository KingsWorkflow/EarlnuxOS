/* ============================================================================
 *  EarlnuxOS - Physical Memory Manager (PMM)
 * mm/pmm.c
 * Buddy system allocator over BIOS/multiboot memory map
 * ============================================================================ */

#include <mm/mm.h>
#include <kernel/kernel.h>

/* ============================================================================
 * Buddy zone state (3 zones: DMA, NORMAL, HIGHMEM)
 * ============================================================================ */
static buddy_zone_t zones[PMM_ZONE_COUNT];
static page_t       page_array[1024 * 256];   /* Max 256K page frames = 1GB */
static uint32_t     total_page_frames = 0;
static uint32_t     phys_memory_end   = 0;

/* PMM initialized flag */
static bool pmm_ready = false;

/* ============================================================================
 * Helpers
 * ============================================================================ */
static inline page_t *pfn_to_page(uint32_t pfn) {
    return &page_array[pfn];
}

static inline uint32_t page_to_pfn(page_t *p) {
    return (uint32_t)(p - page_array);
}

static inline bool pfn_valid(uint32_t pfn) {
    return pfn < total_page_frames;
}

/* Buddy PFN: flip bit at given order */
static inline uint32_t buddy_pfn(uint32_t pfn, uint32_t order) {
    return pfn ^ (1u << order);
}

/* ============================================================================
 * Add a free page range to the buddy allocator
 * ============================================================================ */
static void buddy_free_range(buddy_zone_t *zone, uint32_t start_pfn,
                              uint32_t end_pfn) {
    for (uint32_t pfn = start_pfn; pfn < end_pfn; ) {
        /* Find the largest order block aligned at pfn */
        uint32_t order = PMM_MAX_ORDER;
        while (order > 0) {
            /* Block must be aligned and fit within end_pfn */
            if ((pfn & ((1u << order) - 1)) == 0 &&
                pfn + (1u << order) <= end_pfn)
                break;
            order--;
        }

        page_t *p = pfn_to_page(pfn);
        p->flags = PAGE_FLAG_FREE;
        p->order = order;
        p->next  = zone->free_list[order];
        p->prev  = NULL;
        if (zone->free_list[order])
            zone->free_list[order]->prev = p;
        zone->free_list[order] = p;
        zone->free_count[order]++;
        zone->free_pages += (1u << order);

        pfn += (1u << order);
    }
}

/* ============================================================================
 * Allocate 2^order contiguous page frames from a zone
 * ============================================================================ */
static uint32_t buddy_alloc(buddy_zone_t *zone, uint32_t order) {
    /* Find the smallest available order >= requested */
    uint32_t found_order = order;
    while (found_order <= PMM_MAX_ORDER && !zone->free_list[found_order])
        found_order++;

    if (found_order > PMM_MAX_ORDER) return 0;  /* OOM */

    /* Remove page from free list */
    page_t *p = zone->free_list[found_order];
    zone->free_list[found_order] = p->next;
    if (p->next) p->next->prev = NULL;
    zone->free_count[found_order]--;
    zone->free_pages -= (1u << found_order);

    /* Split down to requested order */
    while (found_order > order) {
        found_order--;
        /* The right half becomes a free buddy */
        uint32_t buddy_pfn_val = page_to_pfn(p) + (1u << found_order);
        if (pfn_valid(buddy_pfn_val)) {
            page_t *buddy = pfn_to_page(buddy_pfn_val);
            buddy->flags = PAGE_FLAG_FREE;
            buddy->order = found_order;
            buddy->next  = zone->free_list[found_order];
            buddy->prev  = NULL;
            if (zone->free_list[found_order])
                zone->free_list[found_order]->prev = buddy;
            zone->free_list[found_order] = buddy;
            zone->free_count[found_order]++;
            zone->free_pages += (1u << found_order);
        }
    }

    p->flags     &= ~PAGE_FLAG_FREE;
    p->order      = order;
    p->ref_count  = 1;
    return pfn_to_phys(page_to_pfn(p));
}

/* ============================================================================
 * Free 2^order contiguous page frames back to a zone
 * ============================================================================ */
static void buddy_free(buddy_zone_t *zone, uint32_t phys, uint32_t order) {
    uint32_t pfn = phys_to_pfn(phys);
    if (!pfn_valid(pfn)) return;

    page_t *p = pfn_to_page(pfn);
    p->ref_count = 0;

    /* Try to merge with buddy */
    while (order < PMM_MAX_ORDER) {
        uint32_t b_pfn = buddy_pfn(pfn, order);
        if (!pfn_valid(b_pfn)) break;

        page_t *buddy = pfn_to_page(b_pfn);
        if (!(buddy->flags & PAGE_FLAG_FREE) || buddy->order != order)
            break;

        /* Remove buddy from free list */
        if (buddy->prev)
            buddy->prev->next = buddy->next;
        else
            zone->free_list[order] = buddy->next;
        if (buddy->next)
            buddy->next->prev = buddy->prev;
        zone->free_count[order]--;
        zone->free_pages -= (1u << order);
        buddy->flags = 0;

        /* Merge: lower PFN is the combined block */
        if (b_pfn < pfn) {
            pfn = b_pfn;
            p   = buddy;
        }
        order++;
    }

    p->flags = PAGE_FLAG_FREE;
    p->order = order;
    p->next  = zone->free_list[order];
    p->prev  = NULL;
    if (zone->free_list[order])
        zone->free_list[order]->prev = p;
    zone->free_list[order] = p;
    zone->free_count[order]++;
    zone->free_pages += (1u << order);
}

/* ============================================================================
 * Zone selection
 * ============================================================================ */
static buddy_zone_t *select_zone(uint32_t phys) {
    if (phys < 16 * 1024 * 1024u)      return &zones[PMM_ZONE_DMA];
    if (phys < 896 * 1024 * 1024u)     return &zones[PMM_ZONE_NORMAL];
    return &zones[PMM_ZONE_HIGHMEM];
}

/* ============================================================================
 * Public PMM API
 * ============================================================================ */
void pmm_init(mem_map_entry_t *map, uint32_t count) {
    /* Initialize zone descriptors */
    zones[PMM_ZONE_DMA].name     = "DMA";
    zones[PMM_ZONE_DMA].base_pfn = 0;
    zones[PMM_ZONE_DMA].end_pfn  = phys_to_pfn(16 * 1024 * 1024u);

    zones[PMM_ZONE_NORMAL].name     = "Normal";
    zones[PMM_ZONE_NORMAL].base_pfn = zones[PMM_ZONE_DMA].end_pfn;
    zones[PMM_ZONE_NORMAL].end_pfn  = phys_to_pfn(896u * 1024 * 1024u);

    zones[PMM_ZONE_HIGHMEM].name     = "HighMem";
    zones[PMM_ZONE_HIGHMEM].base_pfn = zones[PMM_ZONE_NORMAL].end_pfn;
    zones[PMM_ZONE_HIGHMEM].end_pfn  = 0xFFFFFFFFu >> PAGE_SHIFT;

    /* Process memory map */
    for (uint32_t i = 0; i < count; i++) {
        mem_map_entry_t *e = &map[i];
        if (e->type != MEMTYPE_USABLE) continue;
        if (e->base + e->length <= 0x100000u) continue; /* Skip low 1MB */

        uint32_t start = ALIGN_UP((uint32_t)e->base, PAGE_SIZE);
        uint32_t end   = ALIGN_DOWN((uint32_t)(e->base + e->length), PAGE_SIZE);

        if (start < 0x100000u) start = 0x100000u; /* Never below 1MB */
        if (end   <= start)    continue;

        uint32_t pfn_start = phys_to_pfn(start);
        uint32_t pfn_end   = phys_to_pfn(end);

        /* Extend known memory */
        if (end > phys_memory_end) phys_memory_end = end;
        if (pfn_end > total_page_frames) total_page_frames = pfn_end;

        /* Mark page descriptors as usable and add to buddy */
        for (uint32_t pfn = pfn_start; pfn < pfn_end && pfn < ARRAY_SIZE(page_array); pfn++) {
            page_array[pfn].flags = PAGE_FLAG_FREE;
            page_array[pfn].ref_count = 0;
            page_array[pfn].next  = NULL;
            page_array[pfn].prev  = NULL;
        }

        buddy_zone_t *zone = select_zone(start);
        buddy_free_range(zone, pfn_start,
            (pfn_end > zone->end_pfn) ? zone->end_pfn : pfn_end);

        /* Also add to normal zone if spanning */
        if (pfn_end > zones[PMM_ZONE_DMA].end_pfn &&
            pfn_start < zones[PMM_ZONE_NORMAL].end_pfn) {
            uint32_t ns = (pfn_start > zones[PMM_ZONE_DMA].end_pfn)
                          ? pfn_start : zones[PMM_ZONE_DMA].end_pfn;
            uint32_t ne = (pfn_end < zones[PMM_ZONE_NORMAL].end_pfn)
                          ? pfn_end : zones[PMM_ZONE_NORMAL].end_pfn;
            if (ns < ne)
                buddy_free_range(&zones[PMM_ZONE_NORMAL], ns, ne);
        }

        /* Update zone total */
        zone->total_pages += (pfn_end - pfn_start);
    }

    /* Reserve kernel pages (mark as used) */
    /* Kernel image: 1MB - ~2MB typically */
    uint32_t kernel_pfn_start = phys_to_pfn(0x100000u);
    uint32_t kernel_pfn_end   = phys_to_pfn(0x200000u); /* Conservative 1MB kernel size */
    for (uint32_t pfn = kernel_pfn_start; pfn < kernel_pfn_end; pfn++) {
        page_array[pfn].flags = PAGE_FLAG_KERNEL | PAGE_FLAG_RESERVED;
        page_array[pfn].ref_count = 1;
    }

    pmm_ready = true;
    KINFO("pmm", "Physical memory manager online. Total pages: %u, Free: %u",
          total_page_frames,
          zones[PMM_ZONE_NORMAL].free_pages + zones[PMM_ZONE_DMA].free_pages);
}

uint32_t pmm_alloc_page(void) {
    return pmm_alloc_pages(0);
}

uint32_t pmm_alloc_pages(uint32_t order) {
    if (!pmm_ready) return 0;
    /* Try NORMAL first, then DMA, skip HIGHMEM (needs mapping) */
    uint32_t phys = buddy_alloc(&zones[PMM_ZONE_NORMAL], order);
    if (!phys) phys = buddy_alloc(&zones[PMM_ZONE_DMA], order);
    return phys;
}

void pmm_free_page(uint32_t phys) {
    pmm_free_pages(phys, 0);
}

void pmm_free_pages(uint32_t phys, uint32_t order) {
    if (!pmm_ready || !phys) return;
    buddy_zone_t *zone = select_zone(phys);
    buddy_free(zone, phys, order);
}

void pmm_get_stats(pmm_stats_t *s) {
    uint32_t total_free = 0, total_pgs = 0;
    for (int z = 0; z < PMM_ZONE_COUNT; z++) {
        total_free += zones[z].free_pages;
        total_pgs  += zones[z].total_pages;
    }
    s->total_pages   = total_pgs;
    s->free_pages    = total_free;
    s->total_phys    = (uint64_t)total_pgs  * PAGE_SIZE;
    s->usable_phys   = s->total_phys;
    s->free_phys     = (uint64_t)total_free * PAGE_SIZE;
    s->kernel_phys   = (0x200000u - 0x100000u);
    s->reserved_phys = 0x100000u; /* Low 1MB */
}

void pmm_dump_info(void) {
    pmm_stats_t s;
    pmm_get_stats(&s);
    KINFO("pmm", "Memory: %u MB total, %u MB free",
          s.total_phys / (1024*1024), s.free_phys / (1024*1024));
    for (int z = 0; z < PMM_ZONE_COUNT; z++) {
        KINFO("pmm", "  Zone[%s]: total=%u pages, free=%u pages",
              zones[z].name, zones[z].total_pages, zones[z].free_pages);
    }
}
