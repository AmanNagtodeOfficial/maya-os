/**
 * Maya OS Network Interface Card Driver
 * Updated: 2025-08-29 11:11:48 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/nic.h"
#include "drivers/pci.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define NIC_VENDOR_ID_INTEL 0x8086
#define NIC_DEVICE_ID_I825XX 0x100E

#define NIC_REG_CTRL 0x0000
#define NIC_REG_STATUS 0x0008
#define NIC_REG_EEPROM 0x0014
#define NIC_REG_CTRL_EXT 0x0018
#define NIC_REG_RX_CTRL 0x0100
#define NIC_REG_TX_CTRL 0x0400

#define NIC_CTRL_RESET 0x04000000
#define NIC_CTRL_SLU 0x40
#define NIC_CTRL_ASDE 0x20
#define NIC_CTRL_VME 0x40000000

#define NIC_RX_BUFFER_SIZE 2048
#define NIC_TX_BUFFER_SIZE 2048
#define NIC_NUM_RX_DESC 32
#define NIC_NUM_TX_DESC 32

typedef struct {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) nic_rx_desc_t;

typedef struct {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) nic_tx_desc_t;

static struct {
    uint32_t io_base;
    uint8_t mac_addr[6];
    nic_rx_desc_t* rx_descs;
    nic_tx_desc_t* tx_descs;
    void* rx_buffers[NIC_NUM_RX_DESC];
    void* tx_buffers[NIC_NUM_TX_DESC];
    uint32_t rx_cur;
    uint32_t tx_cur;
    nic_rx_callback_t rx_callback;
    bool initialized;
} nic_state;

static uint32_t nic_read_reg(uint16_t reg) {
    return inl(nic_state.io_base + reg);
}

static void nic_write_reg(uint16_t reg, uint32_t value) {
    outl(nic_state.io_base + reg, value);
}

static void nic_handler(struct regs* r) {
    uint32_t status = nic_read_reg(NIC_REG_STATUS);

    // Handle receive
    if (status & 0x80) {
        while (nic_state.rx_descs[nic_state.rx_cur].status & 0x1) {
            uint16_t length = nic_state.rx_descs[nic_state.rx_cur].length;
            
            if (nic_state.rx_callback) {
                nic_state.rx_callback(nic_state.rx_buffers[nic_state.rx_cur], length);
            }

            // Reset descriptor
            nic_state.rx_descs[nic_state.rx_cur].status = 0;
            nic_write_reg(NIC_REG_RX_CTRL + 8, nic_state.rx_cur);

            nic_state.rx_cur = (nic_state.rx_cur + 1) % NIC_NUM_RX_DESC;
        }
    }

    // Handle transmit complete
    if (status & 0x2) {
        while (nic_state.tx_descs[nic_state.tx_cur].status & 0x1) {
            // Reset descriptor
            nic_state.tx_descs[nic_state.tx_cur].status = 0;
            nic_state.tx_cur = (nic_state.tx_cur + 1) % NIC_NUM_TX_DESC;
        }
    }
}

bool nic_init(void) {
    if (nic_state.initialized) {
        return true;
    }

    // Find NIC in PCI
    const pci_device_info_t* dev = NULL;
    for (uint32_t i = 0; i < pci_get_device_count(); i++) {
        dev = pci_get_device(i);
        if (dev->vendor_id == NIC_VENDOR_ID_INTEL &&
            dev->device_id == NIC_DEVICE_ID_I825XX) {
            break;
        }
        dev = NULL;
    }

    if (!dev) {
        return false;
    }

    // Get I/O base address
    nic_state.io_base = dev->bar[0] & ~3;

    // Reset device
    nic_write_reg(NIC_REG_CTRL, NIC_CTRL_RESET);
    while (nic_read_reg(NIC_REG_CTRL) & NIC_CTRL_RESET);

    // Initialize descriptors
    nic_state.rx_descs = kmalloc_aligned(sizeof(nic_rx_desc_t) * NIC_NUM_RX_DESC);
    nic_state.tx_descs = kmalloc_aligned(sizeof(nic_tx_desc_t) * NIC_NUM_TX_DESC);
    if (!nic_state.rx_descs || !nic_state.tx_descs) {
        return false;
    }

    // Initialize buffers
    for (int i = 0; i < NIC_NUM_RX_DESC; i++) {
        nic_state.rx_buffers[i] = kmalloc_aligned(NIC_RX_BUFFER_SIZE);
        if (!nic_state.rx_buffers[i]) {
            return false;
        }
        
        nic_state.rx_descs[i].addr = (uint64_t)nic_state.rx_buffers[i];
        nic_state.rx_descs[i].length = 0;
        nic_state.rx_descs[i].status = 0;
    }

    for (int i = 0; i < NIC_NUM_TX_DESC; i++) {
        nic_state.tx_buffers[i] = kmalloc_aligned(NIC_TX_BUFFER_SIZE);
        if (!nic_state.tx_buffers[i]) {
            return false;
        }
        
        nic_state.tx_descs[i].addr = (uint64_t)nic_state.tx_buffers[i];
        nic_state.tx_descs[i].length = 0;
        nic_state.tx_descs[i].cmd = 0;
        nic_state.tx_descs[i].status = 0;
    }

    // Set up receive control
    nic_write_reg(NIC_REG_RX_CTRL, 
                  (uint64_t)nic_state.rx_descs & 0xFFFFFFFF);
    nic_write_reg(NIC_REG_RX_CTRL + 4, 
                  ((uint64_t)nic_state.rx_descs >> 32) & 0xFFFFFFFF);

    // Set up transmit control
    nic_write_reg(NIC_REG_TX_CTRL, 
                  (uint64_t)nic_state.tx_descs & 0xFFFFFFFF);
    nic_write_reg(NIC_REG_TX_CTRL + 4, 
                  ((uint64_t)nic_state.tx_descs >> 32) & 0xFFFFFFFF);

    // Enable device
    nic_write_reg(NIC_REG_CTRL, NIC_CTRL_SLU | NIC_CTRL_ASDE | NIC_CTRL_VME);

    // Read MAC address
    uint32_t mac_low = nic_read_reg(0x5400);
    uint32_t mac_high = nic_read_reg(0x5404);
    
    nic_state.mac_addr[0] = mac_low & 0xFF;
    nic_state.mac_addr[1] = (mac_low >> 8) & 0xFF;
    nic_state.mac_addr[2] = (mac_low >> 16) & 0xFF;
    nic_state.mac_addr[3] = (mac_low >> 24) & 0xFF;
    nic_state.mac_addr[4] = mac_high & 0xFF;
    nic_state.mac_addr[5] = (mac_high >> 8) & 0xFF;

    // Install interrupt handler
    irq_install_handler(dev->interrupt_line, nic_handler);

    nic_state.initialized = true;
    return true;
}

void nic_set_rx_callback(nic_rx_callback_t callback) {
    if (!nic_state.initialized) {
        return;
    }
    nic_state.rx_callback = callback;
}

bool nic_send_packet(const void* data, size_t length) {
    if (!nic_state.initialized || !data || length == 0 || 
        length > NIC_TX_BUFFER_SIZE) {
        return false;
    }

    // Wait for free descriptor
    while (nic_state.tx_descs[nic_state.tx_cur].status & 0x1);

    // Copy data to buffer
    memcpy(nic_state.tx_buffers[nic_state.tx_cur], data, length);

    // Set up descriptor
    nic_state.tx_descs[nic_state.tx_cur].length = length;
    nic_state.tx_descs[nic_state.tx_cur].cmd = 0x0B;  // EOP + IFCS + RS
    nic_state.tx_descs[nic_state.tx_cur].status = 0;

    // Notify hardware
    nic_write_reg(NIC_REG_TX_CTRL + 8, nic_state.tx_cur);

    nic_state.tx_cur = (nic_state.tx_cur + 1) % NIC_NUM_TX_DESC;
    return true;
}

const uint8_t* nic_get_mac_address(void) {
    if (!nic_state.initialized) {
        return NULL;
    }
    return nic_state.mac_addr;
}

bool nic_is_initialized(void) {
    return nic_state.initialized;
}
