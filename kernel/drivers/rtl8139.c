/* ============================================================================
 *  EarlnuxOS - Realtek 8139 NIC Driver
 * kernel/drivers/rtl8139.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <kernel/pci.h>
#include <kernel/regs.h>
#include <net/net.h>
#include <mm/mm.h>
#include <string.h>

/* RTL8139 Registers */
#define RTL_REG_MAC0        0x00
#define RTL_REG_MAR0        0x08
#define RTL_REG_RBSTART     0x30
#define RTL_REG_CMD         0x37
#define RTL_REG_CAPR        0x38
#define RTL_REG_IMR         0x3C
#define RTL_REG_ISR         0x3E
#define RTL_REG_TCR         0x40
#define RTL_REG_RCR         0x44
#define RTL_REG_CONFIG1     0x52

/* Register bits */
#define CMD_RESET           0x10
#define CMD_RECV_ENABLE     0x08
#define CMD_XMIT_ENABLE     0x04
#define CMD_RX_EMPTY        0x01

#define ISR_ROK             0x01
#define ISR_TOK             0x02

/* Buffer sizes */
#define RX_BUF_SIZE         8192
#define RX_BUF_PAD          16
#define RX_TOTAL_SIZE       (RX_BUF_SIZE + RX_BUF_PAD + 2048) /* Extra for wrap-around */

static uint8_t *rx_buffer;
static uint32_t io_base;
static uint8_t  mac_addr[6];
static netif_t  rtl_netif;

/* Driver state */
static uint32_t rx_offset = 0;

/* External ISR registration */
extern void isr_register_handler(uint8_t num, void (*handler)(struct regs *));

static void rtl8139_handler(struct regs *r) {
    (void)r;
    uint16_t status = inw(io_base + RTL_REG_ISR);
    outw(io_base + RTL_REG_ISR, status); /* Acknowledge */

    if (status & ISR_ROK) {
        /* Packet received */
        while (!(inb(io_base + RTL_REG_CMD) & CMD_RX_EMPTY)) {
            uint16_t *header = (uint16_t *)(rx_buffer + rx_offset);
            uint16_t packet_status = header[0];
            uint16_t packet_len    = header[1];

            if (!(packet_status & 1)) {
                /* Rx error */
                break;
            }

            /* Pass to network stack */
            netif_recv(&rtl_netif, rx_buffer + rx_offset + 4, packet_len - 4);

            /* Update offset (header=4 bytes + packet_len + 4-byte CRC, aligned to 4 bytes) */
            rx_offset = (rx_offset + packet_len + 4 + 3) & ~3;
            rx_offset %= RX_BUF_SIZE;

            outw(io_base + RTL_REG_CAPR, rx_offset - 16);
        }
    }
}

static int rtl8139_transmit(netif_t *iface, const uint8_t *frame, size_t len) {
    (void)iface;
    static int tx_desc = 0;
    static uint8_t *tx_bufs[4] = {NULL, NULL, NULL, NULL};
    
    if (!tx_bufs[0]) {
        for (int i = 0; i < 4; i++) tx_bufs[i] = kmalloc(2048);
    }

    if (len > 1792) return -1;

    memcpy(tx_bufs[tx_desc], frame, len);
    outl(io_base + 0x20 + (tx_desc * 4), (uint32_t)tx_bufs[tx_desc]);
    outl(io_base + 0x10 + (tx_desc * 4), len);

    tx_desc = (tx_desc + 1) % 4;
    return 0;
}

void rtl8139_init(pci_device_t dev) {
    io_base = dev.bar0 & ~3;
    KINFO("RTL8139", "Initializing at I/O base 0x%x\n", io_base);

    /* Power on */
    outb(io_base + RTL_REG_CONFIG1, 0x00);

    /* Software Reset */
    outb(io_base + RTL_REG_CMD, CMD_RESET);
    while (inb(io_base + RTL_REG_CMD) & CMD_RESET);

    /* Read MAC Address */
    for (int i = 0; i < 6; i++) {
        mac_addr[i] = inb(io_base + RTL_REG_MAC0 + i);
    }

    /* Init Receive Buffer */
    rx_buffer = kmalloc(RX_TOTAL_SIZE);
    memset(rx_buffer, 0, RX_TOTAL_SIZE);
    outl(io_base + RTL_REG_RBSTART, (uint32_t)rx_buffer);

    /* Set Interrupt Mask (ROK | TOK) */
    outw(io_base + RTL_REG_IMR, 0x0005);

    /* Configure Receiver */
    outl(io_base + RTL_REG_RCR, 0x8F);

    /* Enable Rx and Tx */
    outb(io_base + RTL_REG_CMD, CMD_RECV_ENABLE | CMD_XMIT_ENABLE);

    /* Register IRQ handler (Remapped IRQs start at 32) */
    isr_register_handler(32 + dev.irq, rtl8139_handler);
    pic_unmask_irq(dev.irq);

    /* Register with network stack */
    memset(&rtl_netif, 0, sizeof(netif_t));
    memcpy(rtl_netif.mac, mac_addr, 6);
    strcpy(rtl_netif.name, "eth0");
    rtl_netif.flags = NETIF_UP;
    rtl_netif.transmit = rtl8139_transmit;
    
    netif_register(&rtl_netif);
}
