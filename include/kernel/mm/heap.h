#ifndef _KERNEL_MM_HEAP_H
#define _KERNEL_MM_HEAP_H

#include <types.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif