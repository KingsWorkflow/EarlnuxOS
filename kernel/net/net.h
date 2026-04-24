/* ============================================================================
 *  EarlnuxOS - Network Stack Header
 * include/net/net.h
 * ============================================================================ */

#ifndef  EarlnuxOS_NET_H
#define  EarlnuxOS_NET_H

#include <types.h>

/* ============================================================================
 * Ethernet / MAC Layer
 * ============================================================================ */
#define ETH_ALEN        6
#define ETH_MTU         1500
#define ETH_FRAME_MAX   1518   /* MTU + 14-byte header + 4-byte FCS */

#define ETH_TYPE_ARP    0x0806
#define ETH_TYPE_IP     0x0800
#define ETH_TYPE_IPV6   0x86DD

typedef struct PACKED eth_header {
    uint8_t  dst[ETH_ALEN];
    uint8_t  src[ETH_ALEN];
    uint16_t type;
} eth_header_t;

/* Broadcast / Multicast addresses */
extern const uint8_t ETH_BROADCAST[ETH_ALEN];  /* FF:FF:FF:FF:FF:FF */

/* ============================================================================
 * IPv4 Layer
 * ============================================================================ */
typedef uint32_t ip4_addr_t;

#define IP4(a,b,c,d) ((uint32_t)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
#define IP4_ANY       IP4(0,0,0,0)
#define IP4_BROADCAST IP4(255,255,255,255)
#define IP4_LOOPBACK  IP4(127,0,0,1)

#define IP_PROTO_ICMP   1
#define IP_PROTO_TCP    6
#define IP_PROTO_UDP    17

typedef struct PACKED ip4_header {
    uint8_t  ihl_ver;       /* [7:4]=version, [3:0]=IHL */
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_off;      /* [15]=DF, [14]=MF, [13:0]=offset */
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    ip4_addr_t src;
    ip4_addr_t dst;
    /* Options (variable) follow here */
} ip4_header_t;

#define IP4_IHL(h)      ((h)->ihl_ver & 0x0F)
#define IP4_VERSION(h)  (((h)->ihl_ver >> 4) & 0x0F)
#define IP4_HDR_LEN(h)  (IP4_IHL(h) * 4)
#define IP4_DF_FLAG     0x4000
#define IP4_MF_FLAG     0x2000
#define IP4_DEFAULT_TTL 64

/* IPv4 API */
void     ip4_init(void);
int      ip4_send(ip4_addr_t dst, uint8_t proto, const void *data, uint16_t len);
void     ip4_recv(const uint8_t *frame, size_t len);
uint16_t ip4_checksum(const void *data, size_t len);

/* IP configuration per interface */
typedef struct ip4_config {
    ip4_addr_t  addr;
    ip4_addr_t  netmask;
    ip4_addr_t  gateway;
    ip4_addr_t  dns[2];
    bool        dhcp_enabled;
} ip4_config_t;

/* ============================================================================
 * ICMP
 * ============================================================================ */
#define ICMP_ECHO_REPLY   0
#define ICMP_ECHO_REQUEST 8
#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEED  11

typedef struct PACKED icmp_header {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
} icmp_header_t;

void icmp_init(void);
int  icmp_ping(ip4_addr_t dst, uint16_t id, uint16_t seq);
void icmp_recv(ip4_addr_t src, const uint8_t *data, size_t len);

/* ============================================================================
 * ARP
 * ============================================================================ */
#define ARP_REQUEST  1
#define ARP_REPLY    2
#define ARP_HW_ETH   1
#define ARP_PRO_IP   0x0800

typedef struct PACKED arp_packet {
    uint16_t hw_type;       /* Hardware type (1=Ethernet) */
    uint16_t pro_type;      /* Protocol type (0x0800=IPv4) */
    uint8_t  hw_size;       /* Hardware address size (6) */
    uint8_t  pro_size;      /* Protocol address size (4) */
    uint16_t opcode;        /* 1=request, 2=reply */
    uint8_t  sender_mac[ETH_ALEN];
    ip4_addr_t sender_ip;
    uint8_t  target_mac[ETH_ALEN];
    ip4_addr_t target_ip;
} arp_packet_t;

#define ARP_CACHE_SIZE  32
#define ARP_TIMEOUT_MS  5000

typedef struct arp_entry {
    ip4_addr_t ip;
    uint8_t    mac[ETH_ALEN];
    uint32_t   timestamp;
    bool       valid;
} arp_entry_t;

void     arp_init(void);
int      arp_request(ip4_addr_t target_ip);
int      arp_lookup(ip4_addr_t ip, uint8_t *mac_out);
void     arp_update(ip4_addr_t ip, const uint8_t *mac);
void     arp_recv(const uint8_t *frame, size_t len);
void     arp_dump_cache(void);

/* ============================================================================
 * UDP
 * ============================================================================ */
typedef struct PACKED udp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} udp_header_t;

#define UDP_MAX_SOCKETS 64

void udp_init(void);
int  udp_send(ip4_addr_t dst, uint16_t src_port, uint16_t dst_port,
              const void *data, size_t len);
void udp_recv(ip4_addr_t src, const uint8_t *data, size_t len);

/* ============================================================================
 * TCP
 * ============================================================================ */
#define TCP_FIN     BIT(0)
#define TCP_SYN     BIT(1)
#define TCP_RST     BIT(2)
#define TCP_PSH     BIT(3)
#define TCP_ACK     BIT(4)
#define TCP_URG     BIT(5)
#define TCP_ECE     BIT(6)
#define TCP_CWR     BIT(7)

typedef struct PACKED tcp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_off;      /* [7:4]=data offset (in 32-bit words) */
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urg_ptr;
} tcp_header_t;

#define TCP_HDR_LEN(h)  (((h)->data_off >> 4) * 4)

/* TCP Connection States */
typedef enum {
    TCP_CLOSED      = 0,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT,
} tcp_state_t;

#define TCP_RECV_BUF_SIZE   (64 * 1024)  /* 64 KB */
#define TCP_SEND_BUF_SIZE   (64 * 1024)
#define TCP_MAX_SOCKETS     256
#define TCP_MSS             1460
#define TCP_WINDOW_SIZE     TCP_RECV_BUF_SIZE

typedef struct tcp_socket {
    tcp_state_t state;
    ip4_addr_t  local_ip;
    ip4_addr_t  remote_ip;
    uint16_t    local_port;
    uint16_t    remote_port;
    uint32_t    snd_una;    /* Oldest unacked sequence number */
    uint32_t    snd_nxt;    /* Next sequence number to send */
    uint32_t    snd_wnd;    /* Send window */
    uint32_t    rcv_nxt;    /* Next expected sequence number */
    uint32_t    rcv_wnd;    /* Receive window */
    uint8_t    *recv_buf;
    uint32_t    recv_head, recv_tail;
    uint8_t    *send_buf;
    uint32_t    send_head, send_tail;
    uint32_t    retransmit_timeout;
    uint32_t    last_activity;
    bool        in_use;
    /* Callback for received data */
    void (*on_data)(struct tcp_socket *sock, const uint8_t *data, size_t len);
    void (*on_close)(struct tcp_socket *sock);
} tcp_socket_t;

/* TCP API */
void          tcp_init(void);
tcp_socket_t *tcp_socket_create(void);
int           tcp_connect(tcp_socket_t *sock, ip4_addr_t dst, uint16_t port);
int           tcp_listen(tcp_socket_t *sock, uint16_t port);
tcp_socket_t *tcp_accept(tcp_socket_t *server);
int           tcp_send(tcp_socket_t *sock, const void *data, size_t len);
int           tcp_recv(tcp_socket_t *sock, void *buf, size_t len);
void          tcp_close(tcp_socket_t *sock);
void          tcp_recv_packet(ip4_addr_t src, const uint8_t *data, size_t len);
const char   *tcp_state_name(tcp_state_t state);

/* ============================================================================
 * DHCP Client
 * ============================================================================ */
#define DHCP_DISCOVER   1
#define DHCP_OFFER      2
#define DHCP_REQUEST    3
#define DHCP_ACK        5
#define DHCP_NAK        6
#define DHCP_RELEASE    7
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

typedef struct dhcp_state {
    bool        configured;
    ip4_addr_t  offered_ip;
    ip4_addr_t  server_ip;
    uint32_t    xid;        /* Transaction ID */
    uint32_t    lease_time;
    uint32_t    renew_time;
} dhcp_state_t;

void dhcp_init(void);
int  dhcp_discover(void);
void dhcp_recv(const uint8_t *data, size_t len);
bool dhcp_is_configured(void);

/* ============================================================================
 * DNS Resolver
 * ============================================================================ */
#define DNS_PORT        53
#define DNS_MAX_NAME    255
#define DNS_CACHE_SIZE  64
#define DNS_TIMEOUT_MS  3000
#define DNS_A_RECORD    1
#define DNS_CNAME       5

typedef struct PACKED dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

#define DNS_QR_FLAG     0x8000  /* Query=0, Response=1 */
#define DNS_OPCODE_MASK 0x7800
#define DNS_AA_FLAG     0x0400
#define DNS_TC_FLAG     0x0200
#define DNS_RD_FLAG     0x0100
#define DNS_RA_FLAG     0x0080

typedef struct dns_cache_entry {
    char       name[DNS_MAX_NAME];
    ip4_addr_t addr;
    uint32_t   ttl;
    uint32_t   timestamp;
    bool       valid;
} dns_cache_entry_t;

void     dns_init(void);
int      dns_resolve(const char *hostname, ip4_addr_t *addr_out);
void     dns_set_server(ip4_addr_t server);
void     dns_cache_dump(void);

/* ============================================================================
 * Network Interface (NIC abstraction)
 * ============================================================================ */
#define NETIF_MAX   4
#define NETIF_UP    BIT(0)
#define NETIF_PROMISC BIT(1)

typedef struct netif {
    char        name[16];
    uint8_t     mac[ETH_ALEN];
    ip4_config_t ip_cfg;
    uint32_t    flags;
    /* Driver callbacks */
    int  (*transmit)(struct netif *iface, const uint8_t *frame, size_t len);
    void (*get_stats)(struct netif *iface, uint64_t *rx_bytes, uint64_t *tx_bytes,
                      uint64_t *rx_pkts, uint64_t *tx_pkts);
    void *driver_data;  /* Opaque driver private data */
    /* Stats */
    uint64_t rx_bytes, tx_bytes;
    uint64_t rx_packets, tx_packets;
    uint64_t rx_errors, tx_errors;
    uint64_t rx_dropped, tx_dropped;
} netif_t;

/* Network interface registry */
void     netif_init(void);
int      netif_register(netif_t *iface);
netif_t *netif_get_default(void);
netif_t *netif_by_name(const char *name);
int      netif_send(netif_t *iface, const void *frame, size_t len);
void     netif_recv(netif_t *iface, const uint8_t *frame, size_t len);
void     netif_list(void);

/* ============================================================================
 * Packet Buffer (sk_buff equivalent)
 * ============================================================================ */
#define SKBUFF_HEADROOM  256
#define SKBUFF_POOL_SIZE 128

typedef struct sk_buff {
    uint8_t *data;          /* Start of actual data */
    uint8_t *head;          /* Start of buffer */
    uint8_t *tail;          /* End of data */
    uint8_t *end;           /* End of buffer */
    size_t   len;           /* Length of data */
    netif_t *dev;           /* Receiving/sending interface */
    struct sk_buff *next;
    /* Layer headers (pointers into data) */
    eth_header_t *eth_hdr;
    ip4_header_t *ip_hdr;
    union {
        tcp_header_t  *tcp_hdr;
        udp_header_t  *udp_hdr;
        icmp_header_t *icmp_hdr;
    };
} sk_buff_t;

sk_buff_t *skb_alloc(size_t size);
void       skb_free(sk_buff_t *skb);
uint8_t   *skb_push(sk_buff_t *skb, size_t len);
uint8_t   *skb_pull(sk_buff_t *skb, size_t len);
uint8_t   *skb_put(sk_buff_t *skb, size_t len);

/* ============================================================================
 * Network stack init
 * ============================================================================ */
void net_init(void);
void net_tick(uint32_t ms);  /* Called from timer interrupt */
void net_dump_stats(void);

/* Byte order conversion */
static inline uint16_t htons(uint16_t v) {
    return (uint16_t)((v >> 8) | (v << 8));
}
static inline uint16_t ntohs(uint16_t v) { return htons(v); }
static inline uint32_t htonl(uint32_t v) {
    return ((v >> 24) & 0xFF)       |
           (((v >> 16) & 0xFF) << 8)  |
           (((v >> 8)  & 0xFF) << 16) |
           ((v & 0xFF) << 24);
}
static inline uint32_t ntohl(uint32_t v) { return htonl(v); }

/* IP address to string (uses static buffer - single use per call) */
const char *ip4_to_str(ip4_addr_t ip);

#endif /*  EarlnuxOS_NET_H */
