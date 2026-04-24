/* IP Stub */
#include <kernel/kernel.h>
void ip4_init(void) { }
int ip4_send(uint32_t dst, uint8_t proto, const void *data, uint16_t len) { return -1; }
void ip4_recv(const uint8_t *frame, size_t len) { }
uint16_t ip4_checksum(const void *data, size_t len) { return 0; }