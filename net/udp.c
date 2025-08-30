/**
 * Maya OS UDP Protocol Implementation
 * Updated: 2025-08-29 11:08:17 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/udp.h"
#include "net/ip.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define UDP_MAX_SOCKETS 256
#define UDP_HEADER_SIZE 8

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
    uint16_t local_port;
    udp_callback_t callback;
    bool in_use;
} udp_socket_t;

static struct {
    udp_socket_t sockets[UDP_MAX_SOCKETS];
    uint16_t next_port;
    bool initialized;
} udp_state;

static uint16_t udp_checksum(uint32_t src_ip, uint32_t dest_ip,
                            const udp_header_t* header,
                            const void* data, size_t length) {
    // UDP checksum includes IP pseudo-header
    uint32_t sum = 0;

    // Add source IP
    sum += (src_ip >> 16) & 0xFFFF;
    sum += src_ip & 0xFFFF;

    // Add destination IP
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;

    // Add protocol and UDP length
    sum += IP_PROTOCOL_UDP;
    sum += header->length;

    // Add UDP header
    const uint16_t* ptr = (const uint16_t*)header;
    for (size_t i = 0; i < sizeof(udp_header_t)/2; i++) {
        if (i != 3) { // Skip checksum field
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

bool udp_init(void) {
    if (udp_state.initialized) {
        return true;
    }

    memset(&udp_state, 0, sizeof(udp_state));
    udp_state.next_port = 49152; // Dynamic port range start
    udp_state.initialized = true;

    // Register with IP protocol handler
    return ip_register_protocol(IP_PROTOCOL_UDP, udp_handle_packet);
}

udp_socket_t* udp_create_socket(uint16_t port, udp_callback_t callback) {
    if (!udp_state.initialized || !callback) {
        return NULL;
    }

    // If port is 0, assign a dynamic port
    if (port == 0) {
        port = udp_state.next_port++;
        if (udp_state.next_port < 49152) {
            udp_state.next_port = 49152;
        }
    }

    // Check if port is already in use
    for (int i = 0; i < UDP_MAX_SOCKETS; i++) {
        if (udp_state.sockets[i].in_use && 
            udp_state.sockets[i].local_port == port) {
            return NULL;
        }
    }

    // Find free socket
    for (int i = 0; i < UDP_MAX_SOCKETS; i++) {
        if (!udp_state.sockets[i].in_use) {
            udp_state.sockets[i].local_port = port;
            udp_state.sockets[i].callback = callback;
            udp_state.sockets[i].in_use = true;
            return &udp_state.sockets[i];
        }
    }

    return NULL;
}

void udp_close_socket(udp_socket_t* socket) {
    if (!udp_state.initialized || !socket) {
        return;
    }

    for (int i = 0; i < UDP_MAX_SOCKETS; i++) {
        if (&udp_state.sockets[i] == socket) {
            memset(socket, 0, sizeof(udp_socket_t));
            break;
        }
    }
}

bool udp_send(const udp_socket_t* socket, uint32_t dest_ip, uint16_t dest_port,
              const void* data, size_t length) {
    if (!udp_state.initialized || !socket || !data || length == 0) {
        return false;
    }

    // Calculate total packet size
    size_t total_length = UDP_HEADER_SIZE + length;
    if (total_length > 65507) { // Max UDP payload size
        return false;
    }

    // Allocate packet buffer
    uint8_t* packet = kmalloc(total_length);
    if (!packet) {
        return false;
    }

    // Build UDP header
    udp_header_t* header = (udp_header_t*)packet;
    header->source_port = ((socket->local_port >> 8) & 0xFF) | 
                         ((socket->local_port & 0xFF) << 8);
    header->dest_port = ((dest_port >> 8) & 0xFF) | 
                       ((dest_port & 0xFF) << 8);
    header->length = ((total_length >> 8) & 0xFF) | 
                    ((total_length & 0xFF) << 8);
    header->checksum = 0;

    // Copy data
    memcpy(packet + UDP_HEADER_SIZE, data, length);

    // Calculate checksum
    header->checksum = udp_checksum(ip_get_address(), dest_ip,
                                  header, data, length);

    // Send through IP
    bool result = ip_send_packet(dest_ip, IP_PROTOCOL_UDP, packet, total_length);

    kfree(packet);
    return result;
}

void udp_handle_packet(uint32_t src_ip, const void* packet, size_t length) {
    if (!udp_state.initialized || !packet || length < UDP_HEADER_SIZE) {
        return;
    }

    const udp_header_t* header = (const udp_header_t*)packet;

    // Convert ports from network byte order
    uint16_t dest_port = ((header->dest_port & 0xFF) << 8) | 
                        ((header->dest_port >> 8) & 0xFF);
    uint16_t src_port = ((header->source_port & 0xFF) << 8) | 
                       ((header->source_port >> 8) & 0xFF);

    // Find matching socket
    for (int i = 0; i < UDP_MAX_SOCKETS; i++) {
        if (udp_state.sockets[i].in_use && 
            udp_state.sockets[i].local_port == dest_port) {
            // Calculate data length
            size_t data_length = length - UDP_HEADER_SIZE;
            const void* data = (uint8_t*)packet + UDP_HEADER_SIZE;

            // Verify checksum
            uint16_t recv_checksum = header->checksum;
            ((udp_header_t*)header)->checksum = 0;
            uint16_t calc_checksum = udp_checksum(src_ip, ip_get_address(),
                                                header, data, data_length);
            if (recv_checksum != calc_checksum) {
                return;
            }

            // Call callback
            udp_state.sockets[i].callback(src_ip, src_port, data, data_length);
            break;
        }
    }
}

bool udp_is_initialized(void) {
    return udp_state.initialized;
}
