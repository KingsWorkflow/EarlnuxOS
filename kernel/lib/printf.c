/* ============================================================================
 *  EarlnuxOS - Simple Printf Implementation
 * kernel/lib/printf.c
 * ============================================================================ */

#include <kernel/console.h>
#include <kernel/kernel.h>
#include <types.h>
#include <lib/string.h>
#include <stdarg.h>

/* ============================================================================
 * Integer → string helpers
 * ============================================================================ */
static void uint_to_str(uint32_t val, char *buf, int base) {
    static const char digits[] = "0123456789abcdef";
    char tmp[32];
    int  i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val) { tmp[i++] = digits[val % (uint32_t)base]; val /= (uint32_t)base; }
    int j = 0;
    while (i--) buf[j++] = tmp[i];
    buf[j] = '\0';
}

static void int_to_str(int32_t val, char *buf) {
    if (val < 0) { *buf++ = '-'; uint_to_str((uint32_t)-val, buf, 10); }
    else         { uint_to_str((uint32_t)val, buf, 10); }
}

/* ============================================================================
 * kvprintf - format string to console using a va_list
 * ============================================================================ */
int kvprintf(const char *fmt, va_list args) {
    char numbuf[32];

    while (*fmt) {
        if (*fmt != '%') {
            console_putchar(*fmt++);
            continue;
        }
        fmt++; /* skip '%' */
        if (!*fmt) break;

        /* Width / precision (skip, not fully implemented) */
        /* Width / precision (skip, not fully implemented) */
        if (*fmt == '-') fmt++;
        while (*fmt >= '0' && *fmt <= '9') fmt++;
        if (*fmt == '.') {
            fmt++;
            while (*fmt >= '0' && *fmt <= '9') fmt++;
        }

        switch (*fmt++) {
        case 'd': case 'i':
            int_to_str(va_arg(args, int), numbuf);
            for (char *p = numbuf; *p; p++) console_putchar(*p);
            break;
        case 'u':
            uint_to_str(va_arg(args, unsigned int), numbuf, 10);
            for (char *p = numbuf; *p; p++) console_putchar(*p);
            break;
        case 'x': case 'X':
            uint_to_str(va_arg(args, unsigned int), numbuf, 16);
            for (char *p = numbuf; *p; p++) console_putchar(*p);
            break;
        case 'p':
            console_putchar('0'); console_putchar('x');
            uint_to_str((uint32_t)(uintptr_t)va_arg(args, void *), numbuf, 16);
            for (char *p = numbuf; *p; p++) console_putchar(*p);
            break;
        case 'c':
            console_putchar((char)va_arg(args, int));
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            if (!s) s = "(null)";
            while (*s) console_putchar(*s++);
            break;
        }
        case '%':
            console_putchar('%');
            break;
        default:
            console_putchar('?');
            break;
        }
    }
    return 0;
}

/* ============================================================================
 * kprintf - variadic wrapper around kvprintf
 * ============================================================================ */
int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = kvprintf(fmt, args);
    va_end(args);
    return r;
}

/* ============================================================================
 * ksprintf / ksnprintf - format into a buffer
 * ============================================================================ */
/* Simple buffer-backed kvprintf helper */
typedef struct { char *p; char *end; } sbuf_t;

static void sbuf_putchar(sbuf_t *b, char c) {
    if (b->p < b->end - 1) *b->p++ = c;
}

static int kvsnprintf(char *buf, size_t n, const char *fmt, va_list args) {
    if (!buf || n == 0) return 0;
    sbuf_t b = { buf, buf + n };
    char numbuf[32];

    while (*fmt) {
        if (*fmt != '%') { sbuf_putchar(&b, *fmt++); continue; }
        fmt++;
        if (!*fmt) break;
        /* Width / precision (skip, not fully implemented) */
        if (*fmt == '-') fmt++;
        while (*fmt >= '0' && *fmt <= '9') fmt++;
        if (*fmt == '.') {
            fmt++;
            while (*fmt >= '0' && *fmt <= '9') fmt++;
        }
        switch (*fmt++) {
        case 'd': case 'i': {
            int32_t v = va_arg(args, int);
            if (v < 0) { sbuf_putchar(&b, '-'); uint_to_str((uint32_t)-v, numbuf, 10); }
            else        { uint_to_str((uint32_t)v, numbuf, 10); }
            for (char *p = numbuf; *p; p++) sbuf_putchar(&b, *p);
            break;
        }
        case 'u':
            uint_to_str(va_arg(args, unsigned int), numbuf, 10);
            for (char *p = numbuf; *p; p++) sbuf_putchar(&b, *p);
            break;
        case 'x': case 'X':
            uint_to_str(va_arg(args, unsigned int), numbuf, 16);
            for (char *p = numbuf; *p; p++) sbuf_putchar(&b, *p);
            break;
        case 'c': sbuf_putchar(&b, (char)va_arg(args, int)); break;
        case 's': {
            const char *s = va_arg(args, const char *);
            if (!s) s = "(null)";
            while (*s) sbuf_putchar(&b, *s++);
            break;
        }
        case '%': sbuf_putchar(&b, '%'); break;
        default:  sbuf_putchar(&b, '?'); break;
        }
    }
    *b.p = '\0';
    return (int)(b.p - buf);
}

int ksprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    /* Use a large sentinel size — caller is responsible for the buffer */
    int r = kvsnprintf(buf, 65536, fmt, args);
    va_end(args);
    return r;
}

int ksnprintf(char *buf, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = kvsnprintf(buf, n, fmt, args);
    va_end(args);
    return r;
}