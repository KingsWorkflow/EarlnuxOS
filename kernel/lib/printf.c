/* ============================================================================
 *  EarlnuxOS - Simple Printf Implementation
 * kernel/lib/printf.c
 * ============================================================================ */

#include <kernel/console.h>
#include <kernel/kernel.h>
#include <types.h>
#include <lib/string.h>

/* Simple itoa for printf */
static char *simple_itoa(int value, char *str, int base) {
    char *ptr = str;
    char *low;
    int temp;

    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }

    low = ptr;

    do {
        temp = value % base;
        *ptr++ = "0123456789abcdef"[temp];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        temp = *low;
        *low++ = *ptr;
        *ptr-- = temp;
    }

    return str;
}

/* Very simple printf for kernel - only supports %d, %u, %x, %s, %c */
int kprintf(const char *fmt, ...) {
    char buf[1024];
    char *str = buf;
    char numbuf[32];
    const char *p = fmt;

    /* Simple varargs simulation - this is not standards compliant but works for kernel */
    void *args = (void*)&fmt + sizeof(const char*);

    while (*p) {
        if (*p != '%') {
            *str++ = *p++;
            continue;
        }

        p++; // Skip %
        if (!*p) break;

        switch (*p++) {
        case 'd': case 'i': {
            int val = *(int*)args;
            args += sizeof(int);
            simple_itoa(val, numbuf, 10);
            char *np = numbuf;
            while (*np) *str++ = *np++;
            break;
        }
        case 'u': {
            unsigned int val = *(unsigned int*)args;
            args += sizeof(unsigned int);
            simple_itoa(val, numbuf, 10);
            char *np = numbuf;
            while (*np) *str++ = *np++;
            break;
        }
        case 'x': case 'X': {
            unsigned int val = *(unsigned int*)args;
            args += sizeof(unsigned int);
            simple_itoa(val, numbuf, 16);
            char *np = numbuf;
            while (*np) *str++ = *np++;
            break;
        }
        case 's': {
            char *val = *(char**)args;
            args += sizeof(char*);
            if (val) {
                while (*val) *str++ = *val++;
            } else {
                char *nil = "(null)";
                while (*nil) *str++ = *nil++;
            }
            break;
        }
        case 'c': {
            char val = *(char*)args;
            args += sizeof(char);
            *str++ = val;
            break;
        }
        case '%': {
            *str++ = '%';
            break;
        }
        default:
            *str++ = '%';
            *str++ = *(p-1);
            break;
        }
    }

    *str = '\0';
    console_puts(buf);
    return (int)(str - buf);
}

/* Stub implementations */
int ksprintf(char *buf, const char *fmt, ...) {
    strlcpy(buf, fmt, 1024); // Just copy format string for now
    return (int)strlen(buf);
}

int ksnprintf(char *buf, size_t n, const char *fmt, ...) {
    strlcpy(buf, fmt, n);
    return (int)strlen(buf);
}