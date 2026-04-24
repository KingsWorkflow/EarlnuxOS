/* ============================================================================
 *  EarlnuxOS - Ethernet Stub
 * kernel/net/eth.c
 * ============================================================================ */

#include <kernel/kernel.h>

void eth_init(void) {
    /* Stub */
}

int eth_send(const void *data, size_t len) {
    return -1; /* ENOSYS */
}

void eth_recv(const void *data, size_t len) {
    /* Stub */
}