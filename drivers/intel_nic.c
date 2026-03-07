/**
 * Maya OS Intel i825xx (e1000) NIC Driver
 * Updated: 2026-03-07 22:30:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/pci.h"
#include "net/nic.h"
#include "kernel/interrupts.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"

#define E1000_DEV_VEN_ID 0x100E8086 // QEMU e1000
#define E1000_REG_CTRL   0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_EEPROM 0x0014
#define E1000_REG_IMASK  0x00D0
#define E1000_REG_RCTL   0x0100
#define E1000_REG_TCTL   0x0400

static struct {
    uint32_t bar_addr;
    uint8_t mac[6];
    bool initialized;
} e1000_state;

static void e1000_write_reg(uint16_t reg, uint32_t value) {
    *(volatile uint32_t*)(e1000_state.bar_addr + reg) = value;
}

static uint32_t e1000_read_reg(uint16_t reg) {
    return *(volatile uint32_t*)(e1000_state.bar_addr + reg);
}

void e1000_init(uint32_t bar, uint8_t irq) {
    e1000_state.bar_addr = bar;
    
    // Read MAC address from EEPROM or registers
    // Simplified for now: use hardcoded or from a known register
    for (int i = 0; i < 6; i++) {
        e1000_state.mac[i] = (uint8_t)((e1000_read_reg(0x5400 + i)) & 0xFF);
    }
    
    // Basic init sequence
    e1000_write_reg(E1000_REG_CTRL, 0x40); // Reset
    
    e1000_state.initialized = true;
    printf("Intel e1000 initialized at BAR 0x%x, IRQ %d\n", bar, irq);
}

const uint8_t* e1000_get_mac(void) {
    return e1000_state.mac;
}

void e1000_send(const void* data, uint16_t length) {
    if (!e1000_state.initialized) return;
    // Stub for actual TX ring buffer logic
}
