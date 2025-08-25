#include "drivers/rtl8139.h"
#include "drivers/pci.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

// RTL8139 register offsets
#define RTL8139_MAC        0x00
#define RTL8139_RBSTART    0x30
#define RTL8139_CMD        0x37
#define RTL8139_IMR        0x3C
#define RTL8139_ISR        0x3E
#define RTL8139_RCR        0x44
#define RTL8139_CONFIG1    0x52

void rtl8139_init(void);
void rtl8139_send_packet(uint8_t *data, uint32_t length);
void rtl8139_receive_handler(struct registers *regs);