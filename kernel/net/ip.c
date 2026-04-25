/* ============================================================================
 *  EarlnuxOS - IPv4 Protocol Layer
 * kernel/net/ip.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>

extern int eth_send(netif_t *iface, const uint8_t *dst_mac, uint16_t type, const uint8_t *data, size_t len);

void ip4_init(void) {
    KINFO("IP", "IPv4 layer initialized\n");
}

void ip4_recv(const uint8_t *frame, size_t len) {
    if (len < sizeof(eth_header_t) + sizeof(ip4_header_t)) return;

    ip4_header_t *ip = (ip4_header_t *)(frame + sizeof(eth_header_t));
    uint8_t version = IP4_VERSION(ip);
    uint8_t ihl = IP4_IHL(ip);

    if (version != 4) return;

    netif_t *iface = netif_get_default();
    if (!iface) return;

    /* Check if for us or broadcast */
    bool for_us = (ip->dst == iface->ip_cfg.addr);
    bool is_bcast = (ip->dst == IP4_BROADCAST);

    if (!for_us && !is_bcast) return;

    size_t header_len = ihl * 4;
    const uint8_t *payload = (const uint8_t *)ip + header_len;
    size_t payload_len = ntohs(ip->total_len) - header_len;

    switch (ip->protocol) {
        case IP_PROTO_ICMP:
            icmp_recv(ip->src, payload, payload_len);
            break;
        case IP_PROTO_UDP:
            udp_recv(ip->src, payload, payload_len);
            break;
        case IP_PROTO_TCP:
            tcp_recv_packet(ip->src, payload, payload_len);
            break;
        default:
            /* Protocol not supported */
            break;
    }
}

int ip4_send(ip4_addr_t dst, uint8_t proto, const void *data, uint16_t len) {
    netif_t *iface = netif_get_default();
    if (!iface) return -1;

    static uint8_t buffer[ETH_MTU];
    ip4_header_t *ip = (ip4_header_t *)buffer;

    ip->ihl_ver = (4 << 4) | 5; /* Version 4, IHL 5 */
    ip->tos = 0;
    ip->total_len = htons(sizeof(ip4_header_t) + len);
    ip->id = htons(0);
    ip->frag_off = htons(IP4_DF_FLAG);
    ip->ttl = IP4_DEFAULT_TTL;
    ip->protocol = proto;
    ip->checksum = 0;
    ip->src = iface->ip_cfg.addr;
    ip->dst = dst;

    /* Checksum (simplified for now, real kernels use optimized asm) */
    // ip->checksum = ip4_checksum(ip, sizeof(ip4_header_t));

    memcpy(buffer + sizeof(ip4_header_t), data, len);

    /* We need the destination MAC. In a real stack, we'd check the ARP cache. 
     * For now, we'll assume broadcast or we'll need to resolve it.
     */
    return eth_send(iface, ETH_BROADCAST, ETH_TYPE_IP, buffer, sizeof(ip4_header_t) + len);
}
