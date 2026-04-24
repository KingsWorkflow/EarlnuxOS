/* ICMP Stub */
#include <kernel/kernel.h>
void icmp_init(void) { }
int icmp_ping(uint32_t dst, uint16_t id, uint16_t seq) { return -1; }
void icmp_recv(uint32_t src, const uint8_t *data, size_t len) { }