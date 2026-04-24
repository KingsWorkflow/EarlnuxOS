/* ============================================================================
 *  EarlnuxOS - x86 CPU Feature Detection
 * include/arch/x86/cpu.h
 * ============================================================================ */

#ifndef  EarlnuxOS_ARCH_X86_CPU_H
#define  EarlnuxOS_ARCH_X86_CPU_H

#include <types.h>

/* CPU feature flags (from CPUID) */
#define CPUID_FPUID    (1 << 0)   /* Floating Point Unit */
#define CPUID_VME      (1 << 1)   /* Virtual 8086 Mode Extensions */
#define CPUID_DE       (1 << 2)   /* Debugging Extensions */
#define CPUID_PSE      (1 << 3)   /* Page Size Extension (4MB pages) */
#define CPUID_TSC      (1 << 4)   /* Time Stamp Counter */
#define CPUID_MSR      (1 << 5)   /* Model-Specific Registers */
#define CPUID_PAE      (1 << 6)   /* Physical Address Extensions (36-bit) */
#define CPUID_MCE      (1 << 7)   /* Machine Check Exception */
#define CPUID_CX8      (1 << 8)   /* CMPXCHG8B instruction */
#define CPUID_APIC     (1 << 9)   /* Local APIC */
#define CPUID_SEP      (1 << 11)  /* SYSENTER/SYSEXIT */
#define CPUID_MTRR     (1 << 12)  /* Memory Type Range Registers */
#define CPUID_PGE      (1 << 13)  /* Page Global Enable */
#define CPUID_MCA      (1 << 14)  /* Machine Check Architecture */
#define CPUID_CMOV     (1 << 15)  /* Conditional Move */
#define CPUID_PAT      (1 << 16)  /* Page Attribute Table */
#define CPUID_PSE36    (1 << 17)  /* 36-bit Page Size Extension */
#define CPUID_PSN      (1 << 18)  /* Processor Serial Number */
#define CPUID_CLFLUSH  (1 << 19)  /* CLFLUSH instruction */
#define CPUID_DS       (1 << 21)  /* Debug Store */
#define CPUID_ACPI     (1 << 22)  /* Thermal Monitor & Clock Ctrl */
#define CPUID_MMX      (1 << 23)  /* MMX instructions */
#define CPUID_FXSR     (1 << 24)  /* FXSAVE/FXRSTOR */
#define CPUID_SSE      (1 << 25)  /* SSE */
#define CPUID_SSE2     (1 << 26)  /* SSE2 */
#define CPUID_SS       (1 << 27)  /* Self Snoop */
#define CPUID_HTT      (1 << 28)  /* Hyper-threading */
#define CPUID_TM       (1 << 29)  /* Thermmonitor */
#define CPUID_IA64     (1 << 30)  /* IA-64 processor */
#define CPUID_PBE      (1u << 31) /* Pending Break Enable */

/* Control Register 0 flags */
#define CR0_PE         (1u << 0)  /* Protection Enable */
#define CR0_MP         (1u << 1)  /* Monitor Coprocessor */
#define CR0_EM         (1u << 2)  /* Emulation */
#define CR0_TS         (1u << 3)  /* Task Switched */
#define CR0_ET         (1u << 4)  /* Extension Type (read-only) */
#define CR0_NE         (1u << 5)  /* Numeric Error */
#define CR0_WP         (1u << 16) /* Write Protect */
#define CR0_AM         (1u << 18) /* Alignment Mask */
#define CR0_NW         (1u << 29) /* Not Write-through */
#define CR0_CD         (1u << 30) /* Cache Disable */
#define CR0_PG         (1u << 31) /* Paging Enable */

/* Control Register 4 (Paging) flags */
#define CR4_VME        (1u << 0)  /* Virtual-8086 Mode Extensions */
#define CR4_PVI        (1u << 1)  /* Protected-Mode Virtual Interrupts */
#define CR4_TSD        (1u << 2)  /* Time Stamp Disable */
#define CR4_DE         (1u << 3)  /* Debugging Extensions */
#define CR4_PSE        (1u << 4)  /* Page Size Extension (4MB pages) */
#define CR4_PAE        (1u << 5)  /* Physical Address Extension (36-bit) */
#define CR4_MCE        (1u << 6)  /* Machine Check Enable */
#define CR4_PGE        (1u << 7)  /* Page Global Enable */
#define CR4_PCE        (1u << 8)  /* Performance Monitoring Counter Enable */
#define CR4_OSFXSR     (1u << 9)  /* OS FXSAVE/FXRSTOR support */
#define CR4_OSXMMEXCPT (1u << 10) /* OS Unmasked SIMD FP Exceptions */

/* EFER (Extended Feature Enable Register) - MSR 0xC0000080 */
#define EFER_SCE       (1u << 0)  /* SysCall Enable (for SYSCALL/SYSRET) */
#define EFER_LME       (1u << 8)  /* Long Mode Enable (for x86_64) */
#define EFER_LMA       (1u << 10) /* Long Mode Active */
#define EFER_NXE       (1u << 11) /* No-Execute Enable (for NX bit) */

/* MSR (Model Specific Registers) addresses */
#define MSR_IA32_SYSENTER_CS  0x174
#define MSR_IA32_SYSENTER_ESP 0x175
#define MSR_IA32_SYSENTER_EIP 0x176
#define MSR_IA32_EFER         0xC0000080

/* CPUID functions */
#define CPUID_FEATURE_ECX    0x00000001
#define CPUID_FEATURE_EDX    0x00000001
#define CPUID_EXT_FEATURE_ECX 0x80000001
#define CPUID_EXT_FEATURE_EDX 0x80000001

/* CPUID info structure */
typedef struct {
    uint32_t eax, ebx, ecx, edx;
} cpuid_t;

/* Detect CPU features at runtime */
bool cpu_has_feature(uint32_t flag);
uint32_t cpu_get_feature_ecx(void);
uint32_t cpu_get_feature_edx(void);
void cpu_detect(void);

/* Read/write control registers (inline assembly wrappers) */
static inline uint32_t read_cr0(void) {
    uint32_t v;
    __asm__ volatile("mov %%cr0, %0" : "=r"(v));
    return v;
}

static inline void write_cr0(uint32_t v) {
    __asm__ volatile("mov %0, %%cr0" : : "r"(v) : "memory");
}

static inline uint32_t read_cr2(void) {
    uint32_t v;
    __asm__ volatile("mov %%cr2, %0" : "=r"(v));
    return v;
}

static inline uint32_t read_cr3(void) {
    uint32_t v;
    __asm__ volatile("mov %%cr3, %0" : "=r"(v));
    return v;
}

static inline void write_cr3(uint32_t v) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(v) : "memory");
}

static inline uint32_t read_cr4(void) {
    uint32_t v;
    __asm__ volatile("mov %%cr4, %0" : "=r"(v));
    return v;
}

static inline void write_cr4(uint32_t v) {
    __asm__ volatile("mov %0, %%cr4" : : "r"(v) : "memory");
}

/* Read Model Specific Register */
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

/* Write Model Specific Register */
static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t lo = (uint32_t)val;
    uint32_t hi = (uint32_t)(val >> 32);
    __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi) : "memory");
}

/* CPUID instruction */
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf));
}

/* Halt the CPU */
static inline void cpu_halt(void) {
    __asm__ volatile("hlt");
}

/* Enable/disable interrupts */
static inline void cpu_sti(void) {
    __asm__ volatile("sti" : : : "memory");
}

static inline void cpu_cli(void) {
    __asm__ volatile("cli" : : : "memory");
}

#endif /*  EarlnuxOS_ARCH_X86_CPU_H */
