/* ============================================================================
 *  EarlnuxOS - Network Stack Master Controller
 * kernel/net/net.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void net_init(void) {
    KINFO("NET", "Initializing Master Network Stack...\n");

    /* Initialize subsystems in order */
    extern void arp_init(void);
    extern void ip4_init(void);
    extern void icmp_init(void);
    extern void udp_init(void);
    extern void tcp_init(void);
    extern void dhcp_init(void);
    extern void dns_init(void);

    arp_init();
    ip4_init();
    icmp_init();
    udp_init();
    tcp_init();
    dhcp_init();
    dns_init();

    KINFO("NET", "Network Stack ready.\n");
}

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

void net_dump_stats(void) {
    kprintf("\nGlobal Network Statistics:\n");
    netif_list();
}

void net_tick(uint32_t ms) {
    (void)ms;
}