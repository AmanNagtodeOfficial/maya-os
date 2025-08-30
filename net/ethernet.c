/**
 * Maya OS Ethernet Protocol Implementation
 * Updated: 2025-08-29 11:03:49 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/ethernet.h"
#include "net/nic.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define ETHERNET_MIN_FRAME_SIZE 60
#define ETHERNET_MAX_FRAME_SIZE 1514
#define ETHERNET_HEADER_SIZE 14

typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} __attribute__((packed)) ethernet_header_t;

static struct {
    uint8_t mac_address[6];
    ethernet_rx_callback_t rx_callback;
    bool initialized;
} ethernet_state;

bool ethernet_init(const uint8_t mac_address[6]) {
    if (ethernet_state.initialized) {
        return true;
    }

    if (!mac_address) {
        return false;
    }

    memcpy(ethernet_state.mac_address, mac_address, 6);
    ethernet_state.rx_callback = NULL;
    ethernet_state.initialized = true;

    return true;
}

void ethernet_set_rx_callback(ethernet_rx_callback_t callback) {
    if (!ethernet_state.initialized) {
        return;
    }

    ethernet_state.rx_callback = callback;
}

bool ethernet_send_frame(const uint8_t dest_mac[6], uint16_t type, const void* data, size_t length) {
    if (!ethernet_state.initialized || !dest_mac || !data || length == 0) {
        return false;
    }

    // Calculate total frame size
    size_t frame_size = ETHERNET_HEADER_SIZE + length;
    if (frame_size < ETHERNET_MIN_FRAME_SIZE) {
        frame_size = ETHERNET_MIN_FRAME_SIZE;
    }

    if (frame_size > ETHERNET_MAX_FRAME_SIZE) {
        return false;
    }

    // Allocate frame buffer
    uint8_t* frame = kmalloc(frame_size);
    if (!frame) {
        return false;
    }

    // Build Ethernet header
    ethernet_header_t* header = (ethernet_header_t*)frame;
    memcpy(header->dest_mac, dest_mac, 6);
    memcpy(header->src_mac, ethernet_state.mac_address, 6);
    header->type = ((type >> 8) & 0xFF) | ((type & 0xFF) << 8);  // Convert to network byte order

    // Copy data
    memcpy(frame + ETHERNET_HEADER_SIZE, data, length);

    // Pad if necessary
    if (frame_size > ETHERNET_HEADER_SIZE + length) {
        memset(frame + ETHERNET_HEADER_SIZE + length, 0, 
               frame_size - (ETHERNET_HEADER_SIZE + length));
    }

    // Send frame through NIC
    bool result = nic_send_packet(frame, frame_size);
    kfree(frame);

    return result;
}

void ethernet_handle_frame(const void* frame, size_t length) {
    if (!ethernet_state.initialized || !frame || 
        length < ETHERNET_HEADER_SIZE || length > ETHERNET_MAX_FRAME_SIZE) {
        return;
    }

    const ethernet_header_t* header = (const ethernet_header_t*)frame;

    // Check if frame is for us (unicast or broadcast)
    bool is_broadcast = true;
    for (int i = 0; i < 6; i++) {
        if (header->dest_mac[i] != 0xFF) {
            is_broadcast = false;
            break;
        }
    }

    if (!is_broadcast) {
        bool is_for_us = true;
        for (int i = 0; i < 6; i++) {
            if (header->dest_mac[i] != ethernet_state.mac_address[i]) {
                is_for_us = false;
                break;
            }
        }
        if (!is_for_us) {
            return;
        }
    }

    // Convert type from network byte order
    uint16_t type = ((header->type & 0xFF) << 8) | ((header->type >> 8) & 0xFF);

    // Pass to callback if registered
    if (ethernet_state.rx_callback) {
        ethernet_state.rx_callback(header->src_mac, type,
                                 (uint8_t*)frame + ETHERNET_HEADER_SIZE,
                                 length - ETHERNET_HEADER_SIZE);
    }
}

const uint8_t* ethernet_get_mac_address(void) {
    if (!ethernet_state.initialized) {
        return NULL;
    }
    return ethernet_state.mac_address;
}

bool ethernet_is_initialized(void) {
    return ethernet_state.initialized;
}
