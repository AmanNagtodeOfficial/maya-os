/**
 * Maya OS RTL8139 Network Driver
 * Updated: 2026-03-07 20:38:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/rtl8139.h"
#include "drivers/pci.h"
#include "kernel/io.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/interrupts.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

// RTL8139 register offsets
#define RTL_REG_MAC0       0x00
#define RTL_REG_RBSTART    0x30
#define RTL_REG_COMMAND    0x37
#define RTL_REG_CAPR       0x38
#define RTL_REG_IMR        0x3C
#define RTL_REG_ISR        0x3E
#define RTL_REG_TCR        0x40
#define RTL_REG_RCR        0x44
#define RTL_REG_CONFIG1    0x52

#define RX_BUF_SIZE 8192

static struct {
    uint32_t io_base;
    uint8_t  mac[6];
    uint8_t* rx_buffer;
    bool     initialized;
} rtl_state;

bool rtl8139_init(void) {
    if (rtl_state.initialized) return true;

    pci_device_t* dev = pci_find_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (!dev) {
        KLOG_W("RTL8139 network card not found.");
        return false;
    }

    rtl_state.io_base = dev->bars[0] & ~0x3;
    
    // Enable PCI bus mastering
    pci_enable_bus_mastering(dev->bus, dev->slot, dev->function);

    // Power on (Config1 = 0)
    outb(rtl_state.io_base + RTL_REG_CONFIG1, 0x00);

    // Software Reset
    outb(rtl_state.io_base + RTL_REG_COMMAND, 0x10);
    while ((inb(rtl_state.io_base + RTL_REG_COMMAND) & 0x10) != 0);

    // Initialise RX buffer
    rtl_state.rx_buffer = kmalloc(RX_BUF_SIZE + 1514 + 16); 
    memset(rtl_state.rx_buffer, 0, RX_BUF_SIZE + 1514 + 16);
    outl(rtl_state.io_base + RTL_REG_RBSTART, (uint32_t)rtl_state.rx_buffer);

    // Set IMR (Interrupt Mask Register) - ROK and TOK
    outw(rtl_state.io_base + RTL_REG_IMR, 0x0005);

    // Set RCR (Receive Configuration Register)
    outl(rtl_state.io_base + RTL_REG_RCR, 0xf | (1 << 7)); 

    // Enable RX and TX
    outb(rtl_state.io_base + RTL_REG_COMMAND, 0x0C);

    // Read MAC address
    for (int i = 0; i < 6; i++) {
        rtl_state.mac[i] = inb(rtl_state.io_base + RTL_REG_MAC0 + i);
    }

    rtl_state.initialized = true;
    KLOG_I("RTL8139 initialized. MAC: %02x:%02x:%02x:%02x:%02x:%02x",
           rtl_state.mac[0], rtl_state.mac[1], rtl_state.mac[2],
           rtl_state.mac[3], rtl_state.mac[4], rtl_state.mac[5]);
    
    return true;
}

void rtl8139_send(uint8_t *data, uint32_t length) {
    if (!rtl_state.initialized) return;

    static uint8_t tx_num = 0;
    uint32_t tx_desc_addr = rtl_state.io_base + 0x20 + (tx_num * 4);
    uint32_t tx_status_addr = rtl_state.io_base + 0x10 + (tx_num * 4);

    outl(tx_desc_addr, (uint32_t)data);
    outl(tx_status_addr, length & 0x1FFF);

    tx_num = (tx_num + 1) % 4;
}

void rtl8139_receive_handler(struct registers *regs) {
    if (!rtl_state.initialized) return;
    (void)regs;
    // Basic handler placeholder - CAPR logic needed for full RX
    uint16_t status = inw(rtl_state.io_base + RTL_REG_ISR);
    outw(rtl_state.io_base + RTL_REG_ISR, status); // Acknowledge
}
const uint8_t* rtl8139_get_mac_address(void) {
    return rtl_state.mac;
}
