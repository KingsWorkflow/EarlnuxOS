/* UDP Stub */
#include <kernel/kernel.h>
void udp_init(void) { }
int udp_send(uint32_t dst, uint16_t dport, const void *data, size_t len) { return -1; }
void udp_recv(uint32_t src, uint16_t sport, const uint8_t *data, size_t len) { }