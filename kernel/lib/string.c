/* ============================================================================
 *  EarlnuxOS - String Library
 * kernel/lib/string.c
 * ============================================================================ */

#include <types.h>

/* Memory operations */
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s && d + n > s) {
        /* Overlap, copy forward */
        while (n--) *d++ = *s++;
    } else if (d > s && s + n > d) {
        /* Overlap, copy backward */
        d += n;
        s += n;
        while (n--) *--d = *--s;
    } else {
        /* No overlap */
        while (n--) *d++ = *s++;
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    char *p = (char *)s;
    while (n--) *p++ = (char)c;
    return s;
}

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    while (n--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = (const unsigned char *)s;
    while (n--) {
        if (*p == (unsigned char)c) return (void *)p;
        p++;
    }
    return NULL;
}

/* String operations */
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    if (n) while (n--) *d++ = '\0';
    return dest;
}

int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && *b && *a == *b) { a++; b++; n--; }
    if (n == 0) return 0;
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

char *strchr(const char *s, int c) {
    while (*s && *s != (char)c) s++;
    return (*s == (char)c) ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == '\0') return (char *)s;
    return (char *)last;
}

size_t strspn(const char *s, const char *accept) {
    size_t count = 0;
    while (*s && strchr(accept, *s++)) count++;
    return count;
}

size_t strcspn(const char *s, const char *reject) {
    const char *p = s;
    while (*p && !strchr(reject, *p)) p++;
    return (size_t)(p - s);
}

char *strpbrk(const char *s, const char *accept) {
    while (*s && !strchr(accept, *s)) s++;
    return (*s) ? (char *)s : NULL;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for ( ; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}

void strrev(char *s) {
    char *end = s + strlen(s) - 1;
    while (s < end) {
        char tmp = *s;
        *s++ = *end;
        *end-- = tmp;
    }
}

/*atoi, atol, etc.*/
int atoi(const char *s) {
    int sign = 1, val = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    return sign * val;
}

long atol(const char *s) {
    long sign = 1, val = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    return sign * val;
}

/* Simple itoa */
static void reverse_str(char *s, int len) {
    for (int i = 0, j = len - 1; i < j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

char *itoa(int value, char *str, int base) {
    int i = 0, sign = 0;
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    if (value < 0 && base == 10) {
        sign = 1;
        value = -value;
    }
    while (value) {
        int digit = value % base;
        str[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    }
    if (sign) str[i++] = '-';
    str[i] = '\0';
    reverse_str(str, i);
    return str;
}

/* BCD to binary and binary to BCD (for RTC) */
uint8_t bcd_to_bin(uint8_t bcd) { return (bcd & 0x0F) + ((bcd >> 4) * 10); }
uint8_t bin_to_bcd(uint8_t bin) { return (uint8_t)(((bin / 10) << 4) | (bin % 10)); }

/* Basic sprintf/snprintf implementation for networking */
int snprintf(char *str, size_t size, const char *format, ...) {
    (void)format;
    // Basic stub implementation - just copy format string
    // In a real kernel, you'd implement proper formatting
    if (!str || size == 0) return 0;

    // Very basic implementation - just null terminate
    // In a real kernel, you'd implement proper formatting later
    str[0] = '\0';
    return 0;
}

// Safe string copy
size_t strlcpy(char *dest, const char *src, size_t size) {
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }

    return ret;
}
