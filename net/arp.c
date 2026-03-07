/**
 * Maya OS ARP Protocol Implementation
 * Updated: 2026-03-07 22:00:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/arp.h"
#include "net/ethernet.h"
#include "net/ip.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2
#define ARP_CACHE_SIZE 64

typedef struct {
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t src_mac[6];
    uint32_t src_ip;
    uint8_t dest_mac[6];
    uint32_t dest_ip;
} __attribute__((packed)) arp_packet_t;

typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    bool occupied;
} arp_entry_t;

static struct {
    arp_entry_t cache[ARP_CACHE_SIZE];
    bool initialized;
} arp_state;

void arp_init(void) {
    memset(&arp_state, 0, sizeof(arp_state));
    arp_state.initialized = true;
}

void arp_resolve(uint32_t ip, uint8_t* mac) {
    if (!arp_state.initialized) return;

    // Check cache
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_state.cache[i].occupied && arp_state.cache[i].ip == ip) {
            memcpy(mac, arp_state.cache[i].mac, 6);
            return;
        }
    }

    // Not in cache, send request and use broadcast for now
    memset(mac, 0xFF, 6);
    arp_send_request(ip);
}

void arp_send_request(uint32_t ip) {
    arp_packet_t packet;
    packet.hardware_type = 0x0100; // Ethernet
    packet.protocol_type = 0x0008; // IP
    packet.hardware_size = 6;
    packet.protocol_size = 4;
    packet.opcode = 0x0100; // Request (network byte order)
    
    extern const uint8_t* nic_get_mac_address(void);
    memcpy(packet.src_mac, nic_get_mac_address(), 6);
    packet.src_ip = ip_get_address();
    
    memset(packet.dest_mac, 0x00, 6);
    packet.dest_ip = ip;

    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ethernet_send_frame(broadcast, 0x0608, &packet, sizeof(arp_packet_t));
}

void arp_handle_packet(const void* packet, size_t length) {
    if (length < sizeof(arp_packet_t)) return;

    const arp_packet_t* arp = (const arp_packet_t*)packet;
    uint16_t opcode = ((arp->opcode & 0xFF) << 8) | (arp->opcode >> 8);

    if (opcode == ARP_OP_REPLY) {
        // Update cache
        for (int i = 0; i < ARP_CACHE_SIZE; i++) {
            if (!arp_state.cache[i].occupied || arp_state.cache[i].ip == arp->src_ip) {
                arp_state.cache[i].ip = arp->src_ip;
                memcpy(arp_state.cache[i].mac, arp->src_mac, 6);
                arp_state.cache[i].occupied = true;
                break;
            }
        }
    } else if (opcode == ARP_OP_REQUEST) {
        if (arp->dest_ip == ip_get_address()) {
            // Send reply
            arp_packet_t reply;
            reply.hardware_type = 0x0100;
            reply.protocol_type = 0x0008;
            reply.hardware_size = 6;
            reply.protocol_size = 4;
            reply.opcode = 0x0200; // Reply
            
            extern const uint8_t* nic_get_mac_address(void);
            memcpy(reply.src_mac, nic_get_mac_address(), 6);
            reply.src_ip = ip_get_address();
            
            memcpy(reply.dest_mac, arp->src_mac, 6);
            reply.dest_ip = arp->src_ip;

            ethernet_send_frame(reply.dest_mac, 0x0608, &reply, sizeof(arp_packet_t));
        }
    }
}
