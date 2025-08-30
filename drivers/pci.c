/**
 * Maya OS PCI Bus Driver
 * Updated: 2025-08-29 11:21:11 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/pci.h"
#include "kernel/io.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_BUSES 256
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_PROG_IF 0x09
#define PCI_SUBCLASS 0x0A
#define PCI_CLASS 0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER 0x0D
#define PCI_HEADER_TYPE 0x0E
#define PCI_BIST 0x0F
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_INTERRUPT_PIN 0x3D

static struct {
    pci_device_info_t devices[PCI_MAX_BUSES * PCI_MAX_DEVICES * PCI_MAX_FUNCTIONS];
    uint32_t device_count;
    bool initialized;
} pci_state;

static uint32_t pci_read_config(uint8_t bus, uint8_t device,
                               uint8_t function, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (function << 8) | (offset & 0xFC) | 0x80000000);
    
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write_config(uint8_t bus, uint8_t device,
                            uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (function << 8) | (offset & 0xFC) | 0x80000000);
    
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_read_config(bus, device, function, PCI_VENDOR_ID) & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return;
    }

    if (pci_state.device_count >= PCI_MAX_BUSES * PCI_MAX_DEVICES * PCI_MAX_FUNCTIONS) {
        return;
    }

    pci_device_info_t* dev = &pci_state.devices[pci_state.device_count++];
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->vendor_id = vendor_id;
    dev->device_id = (pci_read_config(bus, device, function, PCI_DEVICE_ID) >> 16) & 0xFFFF;
    dev->class_code = (pci_read_config(bus, device, function, PCI_CLASS) >> 24) & 0xFF;
    dev->subclass = (pci_read_config(bus, device, function, PCI_SUBCLASS) >> 16) & 0xFF;
    dev->prog_if = (pci_read_config(bus, device, function, PCI_PROG_IF) >> 8) & 0xFF;
    dev->revision_id = pci_read_config(bus, device, function, PCI_REVISION_ID) & 0xFF;
    dev->interrupt_line = (pci_read_config(bus, device, function, PCI_INTERRUPT_LINE) >> 24) & 0xFF;
    dev->interrupt_pin = (pci_read_config(bus, device, function, PCI_INTERRUPT_PIN) >> 16) & 0xFF;

    // Read BAR registers
    for (int i = 0; i < 6; i++) {
        uint32_t bar = pci_read_config(bus, device, function, PCI_BAR0 + i * 4);
        dev->bar[i] = bar;

        if (bar & 1) {
            // I/O Space BAR
            dev->bar_size[i] = 0;
            continue;
        }

        // Memory Space BAR
        if ((bar & 0x6) == 0x4) {
            // 64-bit BAR, skip next one
            i++;
            dev->bar[i] = pci_read_config(bus, device, function, PCI_BAR0 + i * 4);
        }

        // Get BAR size
        pci_write_config(bus, device, function, PCI_BAR0 + i * 4, 0xFFFFFFFF);
        uint32_t size = pci_read_config(bus, device, function, PCI_BAR0 + i * 4);
        pci_write_config(bus, device, function, PCI_BAR0 + i * 4, bar);

        size = ~(size & 0xFFFFFFF0) + 1;
        dev->bar_size[i] = size;
    }
}

static void pci_check_device(uint8_t bus, uint8_t device) {
    uint16_t vendor_id = pci_read_config(bus, device, 0, PCI_VENDOR_ID) & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return;
    }

    pci_check_function(bus, device, 0);

    uint8_t header_type = (pci_read_config(bus, device, 0, PCI_HEADER_TYPE) >> 16) & 0xFF;
    if ((header_type & 0x80) != 0) {
        // Multi-function device
        for (uint8_t function = 1; function < PCI_MAX_FUNCTIONS; function++) {
            if ((pci_read_config(bus, device, function, PCI_VENDOR_ID) & 0xFFFF) != 0xFFFF) {
                pci_check_function(bus, device, function);
            }
        }
    }
}

bool pci_init(void) {
    if (pci_state.initialized) {
        return true;
    }

    memset(&pci_state, 0, sizeof(pci_state));

    // Scan PCI bus
    uint8_t header_type = (pci_read_config(0, 0, 0, PCI_HEADER_TYPE) >> 16) & 0xFF;
    if ((header_type & 0x80) == 0) {
        // Single PCI host controller
        for (uint16_t bus = 0; bus < PCI_MAX_BUSES; bus++) {
            for (uint8_t device = 0; device < PCI_MAX_DEVICES; device++) {
                pci_check_device(bus, device);
            }
        }
    } else {
        // Multiple PCI host controllers
        for (uint8_t function = 0; function < PCI_MAX_FUNCTIONS; function++) {
            if ((pci_read_config(0, 0, function, PCI_VENDOR_ID) & 0xFFFF) != 0xFFFF) {
                for (uint16_t bus = function; bus < PCI_MAX_BUSES; bus++) {
                    for (uint8_t device = 0; device < PCI_MAX_DEVICES; device++) {
                        pci_check_device(bus, device);
                    }
                }
            }
        }
    }

    pci_state.initialized = true;
    return true;
}

uint32_t pci_get_device_count(void) {
    return pci_state.device_count;
}

const pci_device_info_t* pci_get_device(uint32_t index) {
    if (index >= pci_state.device_count) {
        return NULL;
    }
    return &pci_state.devices[index];
}

void pci_enable_bus_mastering(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t command = pci_read_config(bus, device, function, PCI_COMMAND);
    command |= (1 << 2); // Enable Bus Mastering
    pci_write_config(bus, device, function, PCI_COMMAND, command);
}

uint32_t pci_get_bar_address(const pci_device_info_t* dev, uint8_t bar_num) {
    if (!dev || bar_num >= 6) {
        return 0;
    }
    return dev->bar[bar_num] & (dev->bar[bar_num] & 1 ? 0xFFFFFFFC : 0xFFFFFFF0);
}

uint32_t pci_get_bar_size(const pci_device_info_t* dev, uint8_t bar_num) {
    if (!dev || bar_num >= 6) {
        return 0;
    }
    return dev->bar_size[bar_num];
}

bool pci_is_initialized(void) {
    return pci_state.initialized;
}
