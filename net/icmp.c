/**
 * Maya OS ICMP Protocol Implementation
 * Updated: 2025-08-29 11:09:16 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/icmp.h"
#include "net/ip.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define ICMP_HEADER_SIZE 8
#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
} __attribute__((packed)) icmp_header_t;

static struct {
    icmp_callback_t echo_callback;
    bool initialized;
} icmp_state;

static uint16_t icmp_checksum(const void* data, size_t length) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;

    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }

    if (length) {
        sum += *(uint8_t*)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

bool icmp_init(void) {
    if (icmp_state.initialized) {
        return true;
    }

    icmp_state.echo_callback = NULL;
    icmp_state.initialized = true;

    // Register with IP protocol handler
    return ip_register_protocol(IP_PROTOCOL_ICMP, icmp_handle_packet);
}

void icmp_set_echo_callback(icmp_callback_t callback) {
    if (!icmp_state.initialized) {
        return;
    }
    icmp_state.echo_callback = callback;
}

bool icmp_send_echo_request(uint32_t dest_ip, uint16_t identifier, 
                           uint16_t sequence, const void* data, size_t length) {
    if (!icmp_state.initialized || !data || length == 0) {
        return false;
    }

    // Calculate total packet size
    size_t total_length = ICMP_HEADER_SIZE + length;
    
    // Allocate packet buffer
    uint8_t* packet = kmalloc(total_length);
    if (!packet) {
        return false;
    }

    // Build ICMP header
    icmp_header_t* header = (icmp_header_t*)packet;
    header->type = ICMP_ECHO_REQUEST;
    header->code = 0;
    header->checksum = 0;
    header->identifier = ((identifier >> 8) & 0xFF) | ((identifier & 0xFF) << 8);
    header->sequence = ((sequence >> 8) & 0xFF) | ((sequence & 0xFF) << 8);

    // Copy data
    memcpy(packet + ICMP_HEADER_SIZE, data, length);

    // Calculate checksum
    header->checksum = icmp_checksum(packet, total_length);

    // Send through IP
    bool result = ip_send_packet(dest_ip, IP_PROTOCOL_ICMP, packet, total_length);

    kfree(packet);
    return result;
}

void icmp_handle_packet(uint32_t src_ip, const void* packet, size_t length) {
    if (!icmp_state.initialized || !packet || length < ICMP_HEADER_SIZE) {
        return;
    }

    const icmp_header_t* header = (const icmp_header_t*)packet;

    // Verify checksum
    uint16_t recv_checksum = header->checksum;
    ((icmp_header_t*)header)->checksum = 0;
    uint16_t calc_checksum = icmp_checksum(packet, length);
    if (recv_checksum != calc_checksum) {
        return;
    }
    ((icmp_header_t*)header)->checksum = recv_checksum;

    switch (header->type) {
        case ICMP_ECHO_REQUEST: {
            // Send echo reply
            uint8_t* reply = kmalloc(length);
            if (!reply) {
                return;
            }

            // Copy original packet
            memcpy(reply, packet, length);
            icmp_header_t* reply_header = (icmp_header_t*)reply;

            // Modify header for reply
            reply_header->type = ICMP_ECHO_REPLY;
            reply_header->checksum = 0;
            reply_header->checksum = icmp_checksum(reply, length);

            // Send reply
            ip_send_packet(src_ip, IP_PROTOCOL_ICMP, reply, length);
            kfree(reply);
            break;
        }

        case ICMP_ECHO_REPLY: {
            // Convert values from network byte order
            uint16_t identifier = ((header->identifier & 0xFF) << 8) | 
                                ((header->identifier >> 8) & 0xFF);
            uint16_t sequence = ((header->sequence & 0xFF) << 8) | 
                              ((header->sequence >> 8) & 0xFF);

            // Call callback if registered
            if (icmp_state.echo_callback) {
                const void* data = (uint8_t*)packet + ICMP_HEADER_SIZE;
                size_t data_length = length - ICMP_HEADER_SIZE;
                icmp_state.echo_callback(src_ip, identifier, sequence, 
                                       data, data_length);
            }
            break;
        }

        // Handle other ICMP types as needed
    }
}

void icmp_send_destination_unreachable(uint32_t dest_ip, uint8_t code, 
                                     const void* orig_packet, size_t orig_length) {
    if (!icmp_state.initialized || !orig_packet || orig_length < 28) {
        return;
    }

    // Calculate total packet size (8 bytes header + 8 bytes unused + original IP header + 8 bytes of original data)
    size_t total_length = ICMP_HEADER_SIZE + 4 + orig_length;
    if (total_length > 576) { // Maximum recommended ICMP packet size
        total_length = 576;
    }

    // Allocate packet buffer
    uint8_t* packet = kmalloc(total_length);
    if (!packet) {
        return;
    }

    // Build ICMP header
    icmp_header_t* header = (icmp_header_t*)packet;
    header->type = ICMP_DESTINATION_UNREACHABLE;
    header->code = code;
    header->checksum = 0;
    header->identifier = 0;
    header->sequence = 0;

    // Copy original packet data
    memcpy(packet + ICMP_HEADER_SIZE + 4, orig_packet, total_length - ICMP_HEADER_SIZE - 4);

    // Calculate checksum
    header->checksum = icmp_checksum(packet, total_length);

    // Send through IP
    ip_send_packet(dest_ip, IP_PROTOCOL_ICMP, packet, total_length);
    kfree(packet);
}

bool icmp_is_initialized(void) {
    return icmp_state.initialized;
}
