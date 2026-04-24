/* ============================================================================
 *  EarlnuxOS - Network Stack Stub
 * kernel/net/net.c
 * ============================================================================ */

#include <kernel/kernel.h>

/* Stub implementations - return ENOSYS for now */

void net_init(void) {
    KINFO("net", "Network stack initialization (stub)");
}

void net_dump_stats(void) {
    kprintf("Network stats: stub implementation\n");
}

netif_t *netif_get_default(void) {
    return NULL; /* No network interface */
}