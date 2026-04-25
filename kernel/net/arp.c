/* ============================================================================
 *  EarlnuxOS - ARP Protocol Layer
 * kernel/net/arp.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>

static arp_entry_t arp_cache[ARP_CACHE_SIZE];

void arp_init(void) {
    memset(arp_cache, 0, sizeof(arp_cache));
}

void arp_recv(const uint8_t *frame, size_t len) {
    if (len < sizeof(eth_header_t) + sizeof(arp_packet_t)) return;

    arp_packet_t *arp = (arp_packet_t *)(frame + sizeof(eth_header_t));
    netif_t *iface = netif_get_default();
    if (!iface) return;

    uint16_t opcode = ntohs(arp->opcode);

    if (opcode == ARP_REQUEST) {
        /* Check if they are asking for our IP */
        if (arp->target_ip == iface->ip_cfg.addr) {
            KDEBUG("ARP", "Request from %s, sending reply\n", ip4_to_str(arp->sender_ip));
            
            /* Send ARP Reply */
            static uint8_t reply_buf[sizeof(eth_header_t) + sizeof(arp_packet_t)];
            eth_header_t *eth_out = (eth_header_t *)reply_buf;
            arp_packet_t *arp_out = (arp_packet_t *)(reply_buf + sizeof(eth_header_t));

            /* Ethernet header */
            memcpy(eth_out->dst, arp->sender_mac, ETH_ALEN);
            memcpy(eth_out->src, iface->mac, ETH_ALEN);
            eth_out->type = htons(ETH_TYPE_ARP);

            /* ARP Packet */
            arp_out->hw_type = htons(ARP_HW_ETH);
            arp_out->pro_type = htons(ETH_TYPE_IP);
            arp_out->hw_size = ETH_ALEN;
            arp_out->pro_size = 4;
            arp_out->opcode = htons(ARP_REPLY);
            memcpy(arp_out->sender_mac, iface->mac, ETH_ALEN);
            arp_out->sender_ip = iface->ip_cfg.addr;
            memcpy(arp_out->target_mac, arp->sender_mac, ETH_ALEN);
            arp_out->target_ip = arp->sender_ip;

            netif_send(iface, reply_buf, sizeof(reply_buf));
        }
    }
}
