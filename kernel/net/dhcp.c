/* ============================================================================
 *  EarlnuxOS - DHCP Client (Stub)
 * kernel/net/dhcp.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void dhcp_init(void) {
    /* DHCP initialization */
}

int dhcp_discover(void) {
    return 0;
}

void dhcp_recv(const uint8_t *data, size_t len) {
    (void)data;
    (void)len;
}

bool dhcp_is_configured(void) {
    return false;
}
