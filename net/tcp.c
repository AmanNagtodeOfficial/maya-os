/**
 * Maya OS TCP Protocol Implementation
 * Updated: 2025-08-29 11:17:03 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/tcp.h"
#include "net/ip.h"
#include "kernel/memory.h"
#include "kernel/timer.h"
#include "libc/string.h"

#define TCP_MAX_SOCKETS 256
#define TCP_HEADER_SIZE 20
#define TCP_WINDOW_SIZE 8192
#define TCP_MAX_RETRIES 5
#define TCP_TIMEOUT 3000 // milliseconds

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t data_offset;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed)) tcp_header_t;

typedef enum {
    TCP_STATE_CLOSED,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_CLOSING,
    TCP_STATE_LAST_ACK,
    TCP_STATE_TIME_WAIT
} tcp_state_t;

typedef struct tcp_socket {
    uint16_t local_port;
    uint16_t remote_port;
    uint32_t remote_ip;
    tcp_state_t state;
    uint32_t seq_num;
    uint32_t ack_num;
    uint32_t last_seq;
    uint32_t last_ack;
    uint32_t retries;
    uint32_t timeout;
    void* recv_buffer;
    size_t recv_size;
    size_t recv_used;
    tcp_callback_t callback;
    struct tcp_socket* next;
} tcp_socket_t;

static struct {
    tcp_socket_t* sockets;
    uint16_t next_port;
    bool initialized;
} tcp_state;

static uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip,
                           const tcp_header_t* header,
                           const void* data, size_t length) {
    // Calculate TCP pseudo-header checksum
    uint32_t sum = 0;

    // Add source IP
    sum += (src_ip >> 16) & 0xFFFF;
    sum += src_ip & 0xFFFF;

    // Add destination IP
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;

    // Add protocol and TCP length
    sum += IP_PROTOCOL_TCP;
    sum += TCP_HEADER_SIZE + length;

    // Add TCP header
    const uint16_t* ptr = (const uint16_t*)header;
    for (size_t i = 0; i < TCP_HEADER_SIZE/2; i++) {
        if (i != 8) { // Skip checksum field
            sum += ptr[i];
        }
    }

    // Add data
    ptr = (const uint16_t*)data;
    size_t words = length / 2;
    for (size_t i = 0; i < words; i++) {
        sum += ptr[i];
    }

    // Add final byte if length is odd
    if (length & 1) {
        sum += ((uint8_t*)data)[length-1] << 8;
    }

    // Fold 32-bit sum into 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

static void tcp_send_packet(tcp_socket_t* socket, uint8_t flags,
                          const void* data, size_t length) {
    size_t total_size = TCP_HEADER_SIZE + length;
    uint8_t* packet = kmalloc(total_size);
    if (!packet) {
        return;
    }

    // Build TCP header
    tcp_header_t* header = (tcp_header_t*)packet;
    header->source_port = ((socket->local_port >> 8) & 0xFF) |
                         ((socket->local_port & 0xFF) << 8);
    header->dest_port = ((socket->remote_port >> 8) & 0xFF) |
                       ((socket->remote_port & 0xFF) << 8);
    header->seq_num = socket->seq_num;
    header->ack_num = socket->ack_num;
    header->data_offset = (TCP_HEADER_SIZE / 4) << 4;
    header->flags = flags;
    header->window = TCP_WINDOW_SIZE;
    header->checksum = 0;
    header->urgent_ptr = 0;

    // Copy data
    if (data && length > 0) {
        memcpy(packet + TCP_HEADER_SIZE, data, length);
    }

    // Calculate checksum
    header->checksum = tcp_checksum(ip_get_address(), socket->remote_ip,
                                  header, data, length);

    // Send packet
    ip_send_packet(socket->remote_ip, IP_PROTOCOL_TCP, packet, total_size);

    // Update sequence numbers
    if (flags & (TCP_FLAG_SYN | TCP_FLAG_FIN)) {
        socket->seq_num++;
    }
    if (length > 0) {
        socket->seq_num += length;
    }

    kfree(packet);
}

bool tcp_init(void) {
    if (tcp_state.initialized) {
        return true;
    }

    tcp_state.sockets = NULL;
    tcp_state.next_port = 49152; // Dynamic port range start
    tcp_state.initialized = true;

    // Register with IP protocol handler
    return ip_register_protocol(IP_PROTOCOL_TCP, tcp_handle_packet);
}

tcp_socket_t* tcp_create_socket(tcp_callback_t callback) {
    if (!tcp_state.initialized || !callback) {
        return NULL;
    }

    // Allocate socket structure
    tcp_socket_t* socket = kmalloc(sizeof(tcp_socket_t));
    if (!socket) {
        return NULL;
    }

    // Initialize socket
    memset(socket, 0, sizeof(tcp_socket_t));
    socket->local_port = tcp_state.next_port++;
    socket->state = TCP_STATE_CLOSED;
    socket->callback = callback;

    // Add to socket list
    socket->next = tcp_state.sockets;
    tcp_state.sockets = socket;

    return socket;
}

bool tcp_connect(tcp_socket_t* socket, uint32_t ip, uint16_t port) {
    if (!tcp_state.initialized || !socket || 
        socket->state != TCP_STATE_CLOSED) {
        return false;
    }

    socket->remote_ip = ip;
    socket->remote_port = port;
    socket->seq_num = timer_get_ticks(); // Initial sequence number
    socket->state = TCP_STATE_SYN_SENT;

    // Send SYN packet
    tcp_send_packet(socket, TCP_FLAG_SYN, NULL, 0);

    return true;
}

void tcp_close(tcp_socket_t* socket) {
    if (!tcp_state.initialized || !socket) {
        return;
    }

    // Send FIN packet if connected
    if (socket->state == TCP_STATE_ESTABLISHED) {
        tcp_send_packet(socket, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);
        socket->state = TCP_STATE_FIN_WAIT_1;
    } else {
        // Remove from socket list
        if (tcp_state.sockets == socket) {
            tcp_state.sockets = socket->next;
        } else {
            tcp_socket_t* prev = tcp_state.sockets;
            while (prev->next != socket) {
                prev = prev->next;
            }
            prev->next = socket->next;
        }

        kfree(socket);
    }
}

bool tcp_send(tcp_socket_t* socket, const void* data, size_t length) {
    if (!tcp_state.initialized || !socket || !data || length == 0 ||
        socket->state != TCP_STATE_ESTABLISHED) {
        return false;
    }

    tcp_send_packet(socket, TCP_FLAG_PSH | TCP_FLAG_ACK, data, length);
    return true;
}

void tcp_handle_packet(uint32_t src_ip, const void* packet, size_t length) {
    if (!tcp_state.initialized || !packet || length < TCP_HEADER_SIZE) {
        return;
    }

    const tcp_header_t* header = (const tcp_header_t*)packet;

    // Convert ports from network byte order
    uint16_t dest_port = ((header->dest_port & 0xFF) << 8) |
                        ((header->dest_port >> 8) & 0xFF);
    uint16_t src_port = ((header->source_port & 0xFF) << 8) |
                       ((header->source_port >> 8) & 0xFF);

    // Find matching socket
    tcp_socket_t* socket = tcp_state.sockets;
    while (socket) {
        if (socket->local_port == dest_port &&
            (socket->state == TCP_STATE_LISTEN ||
             (socket->remote_port == src_port &&
              socket->remote_ip == src_ip))) {
            break;
        }
        socket = socket->next;
    }

    if (!socket) {
        // No matching socket, send RST
        tcp_send_packet(NULL, TCP_FLAG_RST, NULL, 0);
        return;
    }

    // Handle packet based on current state
    switch (socket->state) {
        case TCP_STATE_LISTEN:
            if (header->flags & TCP_FLAG_SYN) {
                socket->remote_ip = src_ip;
                socket->remote_port = src_port;
                socket->ack_num = header->seq_num + 1;
                socket->seq_num = timer_get_ticks();
                socket->state = TCP_STATE_SYN_RECEIVED;

                // Send SYN-ACK
                tcp_send_packet(socket, TCP_FLAG_SYN | TCP_FLAG_ACK, NULL, 0);
            }
            break;

        case TCP_STATE_SYN_SENT:
            if ((header->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) ==
                (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                socket->ack_num = header->seq_num + 1;
                socket->state = TCP_STATE_ESTABLISHED;

                // Send ACK
                tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);

                // Notify application
                if (socket->callback) {
                    socket->callback(socket, TCP_EVENT_CONNECTED, NULL, 0);
                }
            }
            break;

        case TCP_STATE_ESTABLISHED:
            if (header->flags & TCP_FLAG_PSH) {
                // Handle received data
                const void* data = (uint8_t*)packet + TCP_HEADER_SIZE;
                size_t data_length = length - TCP_HEADER_SIZE;

                socket->ack_num += data_length;

                // Send ACK
                tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);

                // Notify application
                if (socket->callback) {
                    socket->callback(socket, TCP_EVENT_DATA, data, data_length);
                }
            }
            if (header->flags & TCP_FLAG_FIN) {
                socket->ack_num++;
                socket->state = TCP_STATE_CLOSE_WAIT;

                // Send ACK
                tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);

                // Notify application
                if (socket->callback) {
                    socket->callback(socket, TCP_EVENT_CLOSED, NULL, 0);
                }
            }
            break;

        // Handle other states...
    }
}

bool tcp_is_initialized(void) {
    return tcp_state.initialized;
}
