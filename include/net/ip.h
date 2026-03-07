#ifndef NET_IP_H
#define NET_IP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*ip_rx_callback_t)(uint32_t src_ip, const void* data, size_t length);

bool ip_init(uint32_t ip_address, uint32_t subnet_mask, uint32_t gateway);
bool ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, size_t length);
bool ip_register_protocol(uint8_t protocol, ip_rx_callback_t callback);
void ip_handle_packet(const void* packet, size_t length);
uint32_t ip_get_address(void);
uint32_t ip_get_subnet_mask(void);
uint32_t ip_get_gateway(void);
bool ip_is_initialized(void);

#endif
