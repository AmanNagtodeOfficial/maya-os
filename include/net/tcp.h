#ifndef NET_TCP_H
#define NET_TCP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IP_PROTOCOL_TCP 6

#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_RST 0x04
#define TCP_FLAG_PSH 0x08
#define TCP_FLAG_ACK 0x10
#define TCP_FLAG_URG 0x20

typedef enum {
    TCP_EVENT_CONNECTED,
    TCP_EVENT_DATA,
    TCP_EVENT_CLOSED
} tcp_event_t;

typedef struct tcp_socket tcp_socket_t;
typedef void (*tcp_callback_t)(tcp_socket_t* socket, tcp_event_t event, const void* data, size_t length);

bool tcp_init(void);
tcp_socket_t* tcp_create_socket(tcp_callback_t callback);
bool tcp_connect(tcp_socket_t* socket, uint32_t ip, uint16_t port);
void tcp_close(tcp_socket_t* socket);
bool tcp_send(tcp_socket_t* socket, const void* data, size_t length);
void tcp_handle_packet(uint32_t src_ip, const void* packet, size_t length);
bool tcp_is_initialized(void);

#endif
