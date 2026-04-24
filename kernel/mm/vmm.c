/* ============================================================================
 *  EarlnuxOS - Virtual Memory Manager (VMM)
 * mm/vmm.c
 * x86 Two-Level Paging (4KB pages, 4MB pages optional)
 * ============================================================================ */

#include <mm/mm.h>
#include <kernel/kernel.h>

/* ============================================================================
 * Kernel page directory (statically allocated, 4KB aligned)
 * ============================================================================ */
static pgdir_t kernel_pgdir_storage ALIGNED(4096);
pgdir_t *kernel_pgdir = &kernel_pgdir_storage;

/* Active page directory (current process or kernel) */
static pgdir_t *current_pgdir = NULL;

/* ============================================================================
 * CR3 load (activate a page directory)
 * ============================================================================ */
static inline void load_cr3(uint32_t phys) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys) : "memory");
}

static inline void enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;  /* Set PG bit */
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

/* ============================================================================
 * Page table allocation (from PMM)
 * ============================================================================ */
static pgtable_t *alloc_page_table(void) {
    uint32_t phys = pmm_alloc_page();
    if (!phys) return NULL;
    /* Zero it out */
    pgtable_t *pt = (pgtable_t *)phys;
    for (int i = 0; i < 1024; i++) (*pt)[i] = 0;
    return pt;
}

/* ============================================================================
 * Map a single 4KB page
 * ============================================================================ */
int vmm_map_page(pgdir_t *dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pde_idx = PDE_INDEX(virt);
    uint32_t pte_idx = PTE_INDEX(virt);

    pde_t *pde = &(*dir)[pde_idx];

    pgtable_t *pt;
    if (!(*pde & PTE_PRESENT)) {
        /* Allocate a new page table */
        pt = alloc_page_table();
        if (!pt) return -1;
        *pde = ((uint32_t)pt & PTE_ADDR_MASK) | PTE_PRESENT | PTE_WRITE |
               (flags & PTE_USER);
    } else {
        pt = (pgtable_t *)(*pde & PTE_ADDR_MASK);
    }

    /* Set page table entry */
    (*pt)[pte_idx] = (phys & PTE_ADDR_MASK) | PTE_PRESENT |
                     (flags & (PTE_WRITE | PTE_USER | PTE_NOCACHE |
                               PTE_WRITETHROUGH | PTE_GLOBAL));
    vmm_flush_tlb(virt);
    return 0;
}

/* ============================================================================
 * Unmap a single 4KB page
 * ============================================================================ */
void vmm_unmap_page(pgdir_t *dir, uint32_t virt) {
    uint32_t pde_idx = PDE_INDEX(virt);
    uint32_t pte_idx = PTE_INDEX(virt);

    pde_t *pde = &(*dir)[pde_idx];
    if (!(*pde & PTE_PRESENT)) return;

    pgtable_t *pt = (pgtable_t *)(*pde & PTE_ADDR_MASK);
    (*pt)[pte_idx] = 0;
    vmm_flush_tlb(virt);
}

/* ============================================================================
 * Translate virtual to physical address
 * ============================================================================ */
uint32_t vmm_virt_to_phys(pgdir_t *dir, uint32_t virt) {
    uint32_t pde_idx = PDE_INDEX(virt);
    uint32_t pte_idx = PTE_INDEX(virt);

    pde_t pde = (*dir)[pde_idx];
    if (!(pde & PTE_PRESENT)) return 0;

    /* 4MB page? */
    if (pde & PTE_HUGE) {
        return (pde & 0xFFC00000u) | (virt & 0x003FFFFFu);
    }

    pgtable_t *pt = (pgtable_t *)(pde & PTE_ADDR_MASK);
    pte_t pte = (*pt)[pte_idx];
    if (!(pte & PTE_PRESENT)) return 0;

    return (pte & PTE_ADDR_MASK) | (virt & 0xFFFu);
}

/* ============================================================================
 * Map a physical range to a virtual range
 * ============================================================================ */
void vmm_map_range(pgdir_t *dir, uint32_t virt, uint32_t phys,
                   size_t size, uint32_t flags) {
    uint32_t v = ALIGN_DOWN(virt, PAGE_SIZE);
    uint32_t p = ALIGN_DOWN(phys, PAGE_SIZE);
    size_t pages = (ALIGN_UP(size, PAGE_SIZE)) / PAGE_SIZE;

    for (size_t i = 0; i < pages; i++) {
        vmm_map_page(dir, v + i * PAGE_SIZE, p + i * PAGE_SIZE, flags);
    }
}

/* ============================================================================
 * Identity map (virt == phys)
 * ============================================================================ */
void vmm_identity_map(uint32_t start, uint32_t end, uint32_t flags) {
    uint32_t s = ALIGN_DOWN(start, PAGE_SIZE);
    uint32_t e = ALIGN_UP(end, PAGE_SIZE);
    while (s < e) {
        vmm_map_page(kernel_pgdir, s, s, flags);
        s += PAGE_SIZE;
    }
}

/* ============================================================================
 * Clone a page directory (for fork())
 * ============================================================================ */
pgdir_t *vmm_clone_dir(pgdir_t *src) {
    uint32_t phys = pmm_alloc_page();
    if (!phys) return NULL;

    pgdir_t *dst = (pgdir_t *)phys;

    for (int i = 0; i < 1024; i++) {
        pde_t s_pde = (*src)[i];
        if (!(s_pde & PTE_PRESENT)) {
            (*dst)[i] = 0;
            continue;
        }

        if (i >= PDE_INDEX(KERNEL_BASE)) {
            /* Kernel entries: share them */
            (*dst)[i] = s_pde;
            continue;
        }

        /* User entries: copy-on-write (for now, full copy) */
        uint32_t pt_phys = pmm_alloc_page();
        if (!pt_phys) continue;

        pgtable_t *src_pt = (pgtable_t *)(s_pde & PTE_ADDR_MASK);
        pgtable_t *dst_pt = (pgtable_t *)pt_phys;

        for (int j = 0; j < 1024; j++) {
            pte_t s_pte = (*src_pt)[j];
            if (!(s_pte & PTE_PRESENT)) {
                (*dst_pt)[j] = 0;
                continue;
            }
            /* Copy physical page */
            uint32_t new_page = pmm_alloc_page();
            if (!new_page) continue;
            /* memcpy the page */
            uint8_t *sp = (uint8_t *)(s_pte & PTE_ADDR_MASK);
            uint8_t *dp = (uint8_t *)new_page;
            for (int k = 0; k < (int)PAGE_SIZE; k++) dp[k] = sp[k];
            (*dst_pt)[j] = (new_page & PTE_ADDR_MASK) | (s_pte & 0xFFFu);
        }

        (*dst)[i] = (pt_phys & PTE_ADDR_MASK) | (s_pde & 0xFFFu);
    }

    return dst;
}

/* ============================================================================
 * Destroy a page directory (free all user pages and tables)
 * ============================================================================ */
void vmm_destroy_dir(pgdir_t *dir) {
    for (int i = 0; i < PDE_INDEX(KERNEL_BASE); i++) {
        pde_t pde = (*dir)[i];
        if (!(pde & PTE_PRESENT)) continue;
        pgtable_t *pt = (pgtable_t *)(pde & PTE_ADDR_MASK);
        for (int j = 0; j < 1024; j++) {
            pte_t pte = (*pt)[j];
            if (pte & PTE_PRESENT)
                pmm_free_page(pte & PTE_ADDR_MASK);
        }
        pmm_free_page((uint32_t)pt);
    }
    pmm_free_page((uint32_t)dir);
}

/* ============================================================================
 * Switch active page directory
 * ============================================================================ */
void vmm_switch_dir(pgdir_t *dir) {
    current_pgdir = dir;
    load_cr3(vmm_virt_to_phys(kernel_pgdir, (uint32_t)dir));
}

/* ============================================================================
 * TLB invalidation
 * ============================================================================ */
void vmm_flush_tlb(uint32_t virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_flush_tlb_all(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3) : "memory");
}

/* ============================================================================
 * VMM initialization
 * ============================================================================ */
void vmm_init(void) {
    /* Clear kernel page directory */
    for (int i = 0; i < 1024; i++)
        kernel_pgdir_storage[i] = 0;

    /* Identity map the first 8MB (kernel + early allocations) */
    vmm_identity_map(0, 8 * 1024 * 1024, PTE_PRESENT | PTE_WRITE);

    /* Map VGA framebuffer */
    vmm_identity_map(0xB8000, 0xC0000, PTE_PRESENT | PTE_WRITE | PTE_NOCACHE);

    /* Load the kernel page directory */
    uint32_t phys_dir = (uint32_t)&kernel_pgdir_storage;
    load_cr3(phys_dir);
    enable_paging();

    current_pgdir = kernel_pgdir;

    KINFO("vmm", "Paging enabled. Kernel page directory at 0x%08X", phys_dir);
}
