#ifndef _KERNEL_REGS_H
#define _KERNEL_REGS_H

#include <stdint.h>

// Register context structure for interrupt handling
struct regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

#endif