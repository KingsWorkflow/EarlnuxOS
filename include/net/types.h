#ifndef _NET_TYPES_H
#define _NET_TYPES_H

#include <stdint.h>
#include <stddef.h>

// Forward declaration for netif_t
typedef struct netif netif_t;

// Basic network types
typedef uint32_t ip4_addr_t;
typedef uint16_t port_t;

// Packet buffer
typedef struct pbuf {
    struct pbuf *next;
    void *payload;
    size_t len;
    size_t tot_len;
    uint16_t ref;
} pbuf_t;

// Socket structure
typedef struct socket {
    int fd;
    int type;
    ip4_addr_t local_ip;
    port_t local_port;
    ip4_addr_t remote_ip;
    port_t remote_port;
    void *internal;
} socket_t;

// Network stack functions
pbuf_t *net_alloc_pbuf(size_t len);
void net_free_pbuf(pbuf_t *pbuf);
int net_send_packet(netif_t *netif, pbuf_t *pbuf);

#endif