/* TCP Stub */
#include <kernel/kernel.h>
void tcp_init(void) { }
int tcp_connect(uint32_t dst, uint16_t port) { return -1; }
int tcp_send(int sock, const void *data, size_t len) { return -1; }
int tcp_recv(int sock, void *buf, size_t len) { return -1; }
int tcp_close(int sock) { return -1; }