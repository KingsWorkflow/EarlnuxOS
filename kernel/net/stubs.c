#include <net/net.h>
#include <kernel/kernel.h>
#include <string.h>


static ip4_addr_t dns_server = 0;

void dns_set_server(ip4_addr_t server) {
    dns_server = server;
}

int dns_resolve(const char *hostname, ip4_addr_t *addr) {
    if (strcmp(hostname, "localhost") == 0)
        *addr = IP4(127,0,0,1);
    else
        *addr = IP4(8,8,8,8);
    return 0;
}

int dhcp_discover(void) {
    return 0;
}



int ksnprintf(char *str, size_t size, const char *format, ...);

int ip4_parse(const char *str, ip4_addr_t *addr) {
    uint8_t octets[4];
    int i = 0;

    while (*str && i < 4) {
        if (*str < '0' || *str > '9') return -1;

        uint8_t octet = 0;
        while (*str >= '0' && *str <= '9') {
            octet = octet * 10 + (*str - '0');
            str++;
        }

        octets[i++] = octet;

        if (*str == '.') str++;
        else if (*str == '\0' && i == 4) break;
        else return -1;
    }

    if (i != 4) return -1;

    *addr = IP4(octets[0], octets[1], octets[2], octets[3]);
    return 0;
}

const char *ip4_to_str(ip4_addr_t ip) {
    static char buf[16];
    ksnprintf(buf, sizeof(buf), "%u.%u.%u.%u",
             (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF);
    return buf;
}


void arp_dump_cache(void) { KINFO("NET", "ARP stub\n"); }
void netif_list(void) {
    netif_t *iface = netif_get_default();
    if(iface) KINFO("NET", "IFACE: %s MAC: %02x:%02x:%02x:%02x:%02x:%02x IP: %s\n",
        iface->name, iface->mac[0], iface->mac[1], iface->mac[2],
        iface->mac[3], iface->mac[4], iface->mac[5], ip4_to_str(iface->ip_cfg.addr));
}