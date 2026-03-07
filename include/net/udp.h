#ifndef NET_UDP_H
#define NET_UDP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IP_PROTOCOL_UDP 17

typedef struct udp_socket udp_socket_t;
typedef void (*udp_callback_t)(uint32_t src_ip, uint16_t src_port, const void* data, size_t length);

bool udp_init(void);
udp_socket_t* udp_create_socket(uint16_t port, udp_callback_t callback);
void udp_close_socket(udp_socket_t* socket);
bool udp_send(const udp_socket_t* socket, uint32_t dest_ip, uint16_t dest_port, const void* data, size_t length);
void udp_handle_packet(uint32_t src_ip, const void* packet, size_t length);
bool udp_is_initialized(void);

#endif
