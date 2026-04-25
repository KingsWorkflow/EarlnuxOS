/* ============================================================================
 *  EarlnuxOS - DNS Resolver (Stub)
 * kernel/net/dns.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void dns_init(void) {
    /* DNS initialization */
}

int dns_resolve(const char *hostname, ip4_addr_t *addr_out) {
    (void)hostname;
    (void)addr_out;
    return -1;
}

void dns_set_server(ip4_addr_t server) {
    (void)server;
}

void dns_cache_dump(void) {
}
