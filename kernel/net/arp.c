/* ARP Stub */
#include <kernel/kernel.h>
void arp_init(void) { }
void arp_recv(const void *data, size_t len) { }
void arp_dump_cache(void) { }
int arp_resolve(uint32_t ip, uint8_t *mac) { return -1; }