/* ============================================================================
 *  EarlnuxOS - Network Stack Master Controller
 * kernel/net/net.c
 * ============================================================================ */

#include <net/net.h>
#include <kernel/kernel.h>

void net_init(void) {
    KINFO("NET", "Initializing Master Network Stack...\n");
    
    /* Initialize subsystems in order */
    netif_init();
    
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