#ifndef NET_ICMP_H
#define NET_ICMP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IP_PROTOCOL_ICMP 1
#define ICMP_DESTINATION_UNREACHABLE 3

typedef void (*icmp_callback_t)(uint32_t src_ip, uint16_t identifier, uint16_t sequence, const void* data, size_t length);

bool icmp_init(void);
void icmp_set_echo_callback(icmp_callback_t callback);
bool icmp_send_echo_request(uint32_t dest_ip, uint16_t identifier, uint16_t sequence, const void* data, size_t length);
void icmp_handle_packet(uint32_t src_ip, const void* packet, size_t length);
void icmp_send_destination_unreachable(uint32_t dest_ip, uint8_t code, const void* orig_packet, size_t orig_length);
bool icmp_is_initialized(void);

#endif
