/**
 * Maya OS IP Protocol Implementation
 * Updated: 2025-08-29 11:04:56 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/ip.h"
#include "net/ethernet.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define IP_VERSION 4
#define IP_HEADER_LENGTH 20
#define IP_TTL 64
#define IP_TYPE_ICMP 1
#define IP_TYPE_TCP 6
#define IP_TYPE_UDP 17

typedef struct {
    uint8_t header_length:4;
    uint8_t version:4;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

static struct {
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint16_t id_counter;
    ip_rx_callback_t protocol_handlers[256];
    bool initialized;
} ip_state;

static uint16_t ip_checksum(const void* data, size_t length) {
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

bool ip_init(uint32_t ip_address, uint32_t subnet_mask, uint32_t gateway) {
    if (ip_state.initialized) {
        return true;
    }

    ip_state.ip_address = ip_address;
    ip_state.subnet_mask = subnet_mask;
    ip_state.gateway = gateway;
    ip_state.id_counter = 0;

    memset(ip_state.protocol_handlers, 0, sizeof(ip_state.protocol_handlers));
    ip_state.initialized = true;

    return true;
}

bool ip_register_protocol(uint8_t protocol, ip_rx_callback_t callback) {
    if (!ip_state.initialized || !callback) {
        return false;
    }

    ip_state.protocol_handlers[protocol] = callback;
    return true;
}

bool ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, size_t length) {
    if (!ip_state.initialized || !data || length == 0) {
        return false;
    }

    // Calculate total packet size
    size_t total_length = IP_HEADER_LENGTH + length;
    if (total_length > 65535) {
        return false;
    }

    // Allocate packet buffer
    uint8_t* packet = kmalloc(total_length);
    if (!packet) {
        return false;
    }

    // Build IP header
    ip_header_t* header = (ip_header_t*)packet;
    header->version = IP_VERSION;
    header->header_length = IP_HEADER_LENGTH / 4;
    header->tos = 0;
    header->total_length = ((total_length >> 8) & 0xFF) | ((total_length & 0xFF) << 8);
    header->id = ((ip_state.id_counter >> 8) & 0xFF) | ((ip_state.id_counter & 0xFF) << 8);
    header->fragment_offset = 0;
    header->ttl = IP_TTL;
    header->protocol = protocol;
    header->checksum = 0;
    header->src_ip = ip_state.ip_address;
    header->dest_ip = dest_ip;

    // Copy data
    memcpy(packet + IP_HEADER_LENGTH, data, length);

    // Calculate checksum
    header->checksum = ip_checksum(header, IP_HEADER_LENGTH);

    // Determine destination MAC address
    uint8_t dest_mac[6];
    uint32_t next_hop = (dest_ip & ip_state.subnet_mask) == 
                        (ip_state.ip_address & ip_state.subnet_mask) ?
                        dest_ip : ip_state.gateway;

    // TODO: Implement ARP to resolve MAC address
    // For now, use broadcast
    memset(dest_mac, 0xFF, 6);

    // Send through Ethernet
    bool result = ethernet_send_frame(dest_mac, 0x0800, packet, total_length);
    
    kfree(packet);
    ip_state.id_counter++;

    return result;
}

void ip_handle_packet(const void* packet, size_t length) {
    if (!ip_state.initialized || !packet || length < IP_HEADER_LENGTH) {
        return;
    }

    const ip_header_t* header = (const ip_header_t*)packet;

    // Validate header
    if (header->version != IP_VERSION || 
        header->header_length < 5 ||
        length < (header->header_length * 4)) {
        return;
    }

    // Verify checksum
    uint16_t orig_checksum = header->checksum;
    ((ip_header_t*)header)->checksum = 0;
    if (ip_checksum(header, header->header_length * 4) != orig_checksum) {
        return;
    }
    ((ip_header_t*)header)->checksum = orig_checksum;

    // Check if packet is for us
    if (header->dest_ip != ip_state.ip_address) {
        return;
    }

    // Convert lengths from network byte order
    uint16_t total_length = ((header->total_length & 0xFF) << 8) | 
                           ((header->total_length >> 8) & 0xFF);

    // Pass to protocol handler if registered
    if (ip_state.protocol_handlers[header->protocol]) {
        const void* data = (uint8_t*)packet + (header->header_length * 4);
        size_t data_length = total_length - (header->header_length * 4);
        
        ip_state.protocol_handlers[header->protocol](header->src_ip, data, data_length);
    }
}

uint32_t ip_get_address(void) {
    return ip_state.ip_address;
}

uint32_t ip_get_subnet_mask(void) {
    return ip_state.subnet_mask;
}

uint32_t ip_get_gateway(void) {
    return ip_state.gateway;
}

bool ip_is_initialized(void) {
    return ip_state.initialized;
}
