#include "net/nic.h"
#include "drivers/pci.h"
#include "drivers/rtl8139.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "libc/string.h"
#include "kernel/logging.h"

// Intel i825xx constants (keep for fallback)
#define NIC_VENDOR_ID_INTEL 0x8086
#define NIC_DEVICE_ID_I825XX 0x100E

static struct {
    uint32_t io_base;
    uint8_t mac_addr[6];
    bool is_rtl8139;
    bool initialized;
    nic_rx_callback_t rx_callback;
} global_nic_state;

bool nic_init(void) {
    if (global_nic_state.initialized) return true;

    // 1. Try RTL8139
    if (rtl8139_init()) {
        const uint8_t* mac = rtl8139_get_mac_address();
        memcpy(global_nic_state.mac_addr, mac, 6);
        global_nic_state.is_rtl8139 = true;
        global_nic_state.initialized = true;
        KLOG_I("NIC: RTL8139 initialized.");
        return true;
    }

    // 2. Try Intel i825xx (stub logic for now, using existing nic.c logic if needed)
    KLOG_W("NIC: No supported network card found.");
    return false;
}

bool nic_send_packet(const void* data, size_t length) {
    if (!global_nic_state.initialized) return false;

    if (global_nic_state.is_rtl8139) {
        rtl8139_send(data, length);
        return true;
    }
    return false;
}

void nic_set_rx_callback(nic_rx_callback_t callback) {
    global_nic_state.rx_callback = callback;
    // In a real impl, we'd hook this into the driver's handler
}

const uint8_t* nic_get_mac_address(void) {
    return global_nic_state.mac_addr;
}

bool nic_is_initialized(void) {
    return global_nic_state.initialized;
}

