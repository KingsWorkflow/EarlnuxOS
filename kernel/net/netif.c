/* ============================================================================
 *  EarlnuxOS - Network Interface Management
 * kernel/net/netif.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>
#include <mm/mm.h>

static netif_t *interfaces[NETIF_MAX];
static int netif_count = 0;
static netif_t *default_iface = NULL;

void netif_init(void) {
    memset(interfaces, 0, sizeof(interfaces));
    netif_count = 0;
    default_iface = NULL;
    KINFO("NET", "Network interface layer initialized\n");
}

int netif_register(netif_t *iface) {
    if (netif_count >= NETIF_MAX) return -1;
    
    interfaces[netif_count++] = iface;
    if (!default_iface) default_iface = iface;
    
    KINFO("NET", "Registered interface '%s' (MAC: %02x:%02x:%02x:%02x:%02x:%02x)\n",
          iface->name, iface->mac[0], iface->mac[1], iface->mac[2],
          iface->mac[3], iface->mac[4], iface->mac[5]);
          
    return 0;
}

netif_t *netif_get_default(void) {
    return default_iface;
}

netif_t *netif_by_name(const char *name) {
    for (int i = 0; i < netif_count; i++) {
        if (strcmp(interfaces[i]->name, name) == 0) return interfaces[i];
    }
    return NULL;
}

int netif_send(netif_t *iface, const void *frame, size_t len) {
    if (!iface || !iface->transmit) return -1;
    if (!(iface->flags & NETIF_UP)) return -1;
    
    int ret = iface->transmit(iface, frame, len);
    if (ret == 0) {
        iface->tx_packets++;
        iface->tx_bytes += len;
    } else {
        iface->tx_errors++;
    }
    return ret;
}

extern void eth_recv(netif_t *iface, const uint8_t *frame, size_t len);

void netif_recv(netif_t *iface, const uint8_t *frame, size_t len) {
    if (!iface || !frame) return;
    
    iface->rx_packets++;
    iface->rx_bytes += len;
    
    /* Hand off to Ethernet layer */
    eth_recv(iface, frame, len);
}

void netif_list(void) {
    for (int i = 0; i < netif_count; i++) {
        netif_t *iface = interfaces[i];
        kprintf("%-8s HWaddr %02x:%02x:%02x:%02x:%02x:%02x\n",
                iface->name, iface->mac[0], iface->mac[1], iface->mac[2],
                iface->mac[3], iface->mac[4], iface->mac[5]);
        kprintf("         inet addr:%s  Mask:%s\n",
                ip4_to_str(iface->ip_cfg.addr), ip4_to_str(iface->ip_cfg.netmask));
        kprintf("         RX packets:%u bytes:%u  TX packets:%u bytes:%u\n\n",
                (uint32_t)iface->rx_packets, (uint32_t)iface->rx_bytes,
                (uint32_t)iface->tx_packets, (uint32_t)iface->tx_bytes);
    }
}
