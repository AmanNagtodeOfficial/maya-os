#ifndef NET_NIC_H
#define NET_NIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*nic_rx_callback_t)(const void* data, size_t length);

bool nic_init(void);
bool nic_send_packet(const void* data, size_t length);
void nic_set_rx_callback(nic_rx_callback_t callback);
const uint8_t* nic_get_mac_address(void);
bool nic_is_initialized(void);

#endif
