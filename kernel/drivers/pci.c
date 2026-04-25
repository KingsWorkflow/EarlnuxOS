/* ============================================================================
 *  EarlnuxOS - PCI Bus Scanner
 * kernel/drivers/pci.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <kernel/pci.h>

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* Create configuration address */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* Write out the address */
    outl(PCI_CONFIG_ADDRESS, address);
    /* Read in the data */
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

extern void rtl8139_init(pci_device_t device);

void pci_init(void) {
    KINFO("PCI", "Scanning PCI bus...\n");

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_config_read_dword(bus, slot, func, 0);
                if ((vendor_device & 0xFFFF) == 0xFFFF) continue;

                pci_device_t dev;
                dev.bus = bus;
                dev.device = slot;
                dev.function = func;
                dev.vendor_id = vendor_device & 0xFFFF;
                dev.device_id = (vendor_device >> 16) & 0xFFFF;
                
                uint32_t class_rev = pci_config_read_dword(bus, slot, func, 0x08);
                dev.class_id = (class_rev >> 16);
                
                dev.bar0 = pci_config_read_dword(bus, slot, func, PCI_BAR0);
                uint32_t irq_info = pci_config_read_dword(bus, slot, func, PCI_INTERRUPT_LINE);
                dev.irq = irq_info & 0xFF;

                /* Log discovery */
                KDEBUG("PCI", "Found [%04x:%04x] at %02x:%02x.%d (IRQ %d)\n", 
                       dev.vendor_id, dev.device_id, bus, slot, func, dev.irq);

                /* Realtek 8139 */
                if (dev.vendor_id == 0x10EC && dev.device_id == 0x8139) {
                    KINFO("PCI", "Detected RTL8139 Network Controller\n");
                    rtl8139_init(dev);
                }
                
                /* Intel E1000 */
                if (dev.vendor_id == 0x8086 && (dev.device_id == 0x100E || dev.device_id == 0x10D3)) {
                    KINFO("PCI", "Detected Intel E1000 Network Controller\n");
                    /* Intel driver pending... */
                }
            }
        }
    }
}
