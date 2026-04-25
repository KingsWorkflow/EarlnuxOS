/* ============================================================================
 *  EarlnuxOS - UDP Protocol Layer (Stub)
 * kernel/net/udp.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void udp_init(void) {
    /* UDP initialization logic */
}

void udp_recv(ip4_addr_t src, const uint8_t *data, size_t len) {
    (void)src;
    (void)data;
    (void)len;
    /* Future: dispatch to registered UDP sockets */
}
