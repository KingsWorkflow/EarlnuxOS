/* ============================================================================
 *  EarlnuxOS - PCI Bus Header
 * include/kernel/pci.h
 * ============================================================================ */

#ifndef  EarlnuxOS_PCI_H
#define  EarlnuxOS_PCI_H

#include <types.h>

/* PCI configuration ports */
#define PCI_CONFIG_ADDRESS  0x0CF8
#define PCI_CONFIG_DATA     0x0CFC

/* PCI header offsets */
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_REVISION_ID     0x08
#define PCI_PROG_IF         0x09
#define PCI_SUBCLASS        0x0A
#define PCI_CLASS           0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER   0x0D
#define PCI_HEADER_TYPE     0x0E
#define PCI_BIST            0x0F
#define PCI_BAR0            0x10
#define PCI_INTERRUPT_LINE  0x3C

typedef struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t class_id;
    uint32_t bar0;
    uint8_t irq;
} pci_device_t;

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
void pci_init(void);

#endif /*  EarlnuxOS_PCI_H */
