#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*ethernet_rx_callback_t)(const uint8_t src_mac[6], uint16_t type, const void* data, size_t length);

bool ethernet_init(const uint8_t mac_address[6]);
bool ethernet_send_frame(const uint8_t dest_mac[6], uint16_t type, const void* data, size_t length);
void ethernet_set_rx_callback(ethernet_rx_callback_t callback);
const uint8_t* ethernet_get_mac_address(void);
bool ethernet_is_initialized(void);
void ethernet_handle_frame(const void* frame, size_t length);

#endif
