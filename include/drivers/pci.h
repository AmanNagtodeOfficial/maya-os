#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_VENDOR_ID      0x00
#define PCI_DEVICE_ID      0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_CLASS_CODE     0x0B
#define PCI_HEADER_TYPE    0x0E
#define PCI_BAR0           0x10
#define PCI_BAR1           0x14

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id; 
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint32_t bars[6];
} pci_device_t;

void pci_init(void);
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);

#endif