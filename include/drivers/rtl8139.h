#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>
#include <stdbool.h>

bool rtl8139_init(void);
const uint8_t* rtl8139_get_mac_address(void);
void rtl8139_send(const void* data, uint32_t length);
void rtl8139_recv(void);
struct registers;
void rtl8139_receive_handler(struct registers *regs);

#endif
