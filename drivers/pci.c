#include "drivers/pci.h"
#include "kernel/kernel.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// PCI device structure
typedef struct {
    uint16_t vendor_id;
        uint16_t device_id;
            uint8_t class_code;
                uint8_t subclass;
                    uint8_t bus;
                        uint8_t slot;
                            uint8_t func;
                            } pci_device_t;

                            void pci_init(void);
                            uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
                            void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
                            pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
                            void pci_enumerate_devices(void);