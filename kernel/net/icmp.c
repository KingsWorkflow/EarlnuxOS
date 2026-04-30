/* ============================================================================
 *  EarlnuxOS - ICMP Protocol Layer
 * kernel/net/icmp.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>

/* Forward declarations */
uint16_t icmp_checksum(const void *data, size_t len);

void icmp_init(void) {
    KINFO("ICMP", "ICMP layer initialized\n");
}

int icmp_ping(ip4_addr_t dst, uint16_t id, uint16_t seq) {
    static uint8_t ping_buf[64];
    icmp_header_t *icmp = (icmp_header_t *)ping_buf;

    icmp->type = ICMP_ECHO_REQUEST;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->id = id;
    icmp->seq = seq;

    /* Add simple payload */
    uint8_t *payload = ping_buf + sizeof(icmp_header_t);
    for (int i = 0; i < 32; i++) {
        payload[i] = 'A' + (i % 26);
    }

    size_t len = sizeof(icmp_header_t) + 32;
    icmp->checksum = icmp_checksum(ping_buf, len);

    KDEBUG("ICMP", "Sending ping to %s (id=%u, seq=%u)\n",
           ip4_to_str(dst), id, seq);

    return ip4_send(dst, IP_PROTO_ICMP, ping_buf, len);
}

uint16_t icmp_checksum(const void *data, size_t len) {
    uint32_t sum = 0;
    const uint16_t *p = (const uint16_t *)data;
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    if (len) sum += *(const uint8_t *)p;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~((uint16_t)sum);
}

void icmp_recv(ip4_addr_t src, const uint8_t *data, size_t len) {
    if (len < sizeof(icmp_header_t)) return;

    icmp_header_t *icmp = (icmp_header_t *)data;

    if (icmp->type == ICMP_ECHO_REQUEST) {
        KDEBUG("ICMP", "Ping request from %s\n", ip4_to_str(src));

        /* Send Echo Reply */
        static uint8_t reply_buf[1024];
        if (len > sizeof(reply_buf)) len = sizeof(reply_buf);

        icmp_header_t *reply = (icmp_header_t *)reply_buf;
        reply->type = ICMP_ECHO_REPLY;
        reply->code = 0;
        reply->checksum = 0;
        reply->id = icmp->id;
        reply->seq = icmp->seq;

        /* Copy payload if any */
        if (len > sizeof(icmp_header_t)) {
            memcpy(reply_buf + sizeof(icmp_header_t), 
                   data + sizeof(icmp_header_t), 
                   len - sizeof(icmp_header_t));
        }

        reply->checksum = icmp_checksum(reply_buf, len);

        ip4_send(src, IP_PROTO_ICMP, reply_buf, len);
    }
}
