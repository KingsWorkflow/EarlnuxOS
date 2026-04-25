/* ============================================================================
 *  EarlnuxOS - TCP Protocol Layer (Stub)
 * kernel/net/tcp.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void tcp_init(void) {
    /* TCP initialization logic */
}

void tcp_recv_packet(ip4_addr_t src, const uint8_t *data, size_t len) {
    (void)src;
    (void)data;
    (void)len;
    /* Future: handle TCP state machine */
}

const char *tcp_state_name(tcp_state_t state) {
    (void)state;
    return "CLOSED";
}
