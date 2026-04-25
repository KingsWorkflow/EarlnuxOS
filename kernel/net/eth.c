/* ============================================================================
 *  EarlnuxOS - Ethernet Protocol Layer
 * kernel/net/eth.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>

const uint8_t ETH_BROADCAST[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void eth_recv(netif_t *iface, const uint8_t *frame, size_t len) {
    if (len < sizeof(eth_header_t)) return;

    eth_header_t *hdr = (eth_header_t *)frame;
    uint16_t type = ntohs(hdr->type);

    /* Check if for us or broadcast */
    bool for_us = (memcmp(hdr->dst, iface->mac, ETH_ALEN) == 0);
    bool is_bcast = (memcmp(hdr->dst, ETH_BROADCAST, ETH_ALEN) == 0);

    if (!for_us && !is_bcast) return;

    (void)frame; (void)len;

    switch (type) {
        case ETH_TYPE_ARP:
            arp_recv(frame, len);
            break;
        case ETH_TYPE_IP:
            ip4_recv(frame, len);
            break;
        default:
            /* Unknown ethertype */
            break;
    }
}

int eth_send(netif_t *iface, const uint8_t *dst_mac, uint16_t type, const uint8_t *data, size_t len) {
    if (!iface || len > ETH_MTU) return -1;

    static uint8_t buffer[ETH_FRAME_MAX];
    eth_header_t *hdr = (eth_header_t *)buffer;

    memcpy(hdr->dst, dst_mac, ETH_ALEN);
    memcpy(hdr->src, iface->mac, ETH_ALEN);
    hdr->type = htons(type);

    memcpy(buffer + sizeof(eth_header_t), data, len);

    return netif_send(iface, buffer, len + sizeof(eth_header_t));
}
