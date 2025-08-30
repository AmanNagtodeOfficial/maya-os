/**
 * Maya OS DHCP Client Implementation
 * Updated: 2025-08-29 11:19:39 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/dhcp.h"
#include "net/udp.h"
#include "net/ip.h"
#include "kernel/timer.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define DHCP_MAGIC_COOKIE 0x63825363

#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY 2

#define DHCP_OPTION_PAD 0
#define DHCP_OPTION_SUBNET_MASK 1
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_DNS_SERVER 6
#define DHCP_OPTION_REQUESTED_IP 50
#define DHCP_OPTION_LEASE_TIME 51
#define DHCP_OPTION_MESSAGE_TYPE 53
#define DHCP_OPTION_SERVER_ID 54
#define DHCP_OPTION_PARAMETER_REQUEST 55
#define DHCP_OPTION_END 255

#define DHCP_MESSAGE_DISCOVER 1
#define DHCP_MESSAGE_OFFER 2
#define DHCP_MESSAGE_REQUEST 3
#define DHCP_MESSAGE_ACK 5
#define DHCP_MESSAGE_NAK 6

typedef struct {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic_cookie;
    uint8_t options[];
} __attribute__((packed)) dhcp_packet_t;

static struct {
    udp_socket_t* socket;
    uint32_t xid;
    uint32_t server_ip;
    uint32_t offered_ip;
    uint32_t lease_time;
    dhcp_callback_t callback;
    bool initialized;
} dhcp_state;

static void dhcp_add_option(uint8_t* options, size_t* offset,
                           uint8_t type, uint8_t length, const void* data) {
    options[(*offset)++] = type;
    options[(*offset)++] = length;
    memcpy(&options[*offset], data, length);
    *offset += length;
}

static bool dhcp_get_option(const uint8_t* options, size_t options_len,
                           uint8_t type, void* data, size_t* length) {
    size_t offset = 0;

    while (offset < options_len) {
        uint8_t option_type = options[offset++];

        if (option_type == DHCP_OPTION_END) {
            break;
        }

        if (option_type == DHCP_OPTION_PAD) {
            continue;
        }

        uint8_t option_length = options[offset++];
        if (offset + option_length > options_len) {
            break;
        }

        if (option_type == type) {
            if (data && length && *length >= option_length) {
                memcpy(data, &options[offset], option_length);
                *length = option_length;
                return true;
            }
            return false;
        }

        offset += option_length;
    }

    return false;
}

static void dhcp_handle_packet(uint32_t src_ip, uint16_t src_port,
                             const void* data, size_t length) {
    if (length < sizeof(dhcp_packet_t)) {
        return;
    }

    const dhcp_packet_t* packet = (const dhcp_packet_t*)data;

    // Verify packet
    if (packet->op != DHCP_OP_REPLY ||
        packet->xid != dhcp_state.xid ||
        packet->magic_cookie != DHCP_MAGIC_COOKIE) {
        return;
    }

    // Get message type
    uint8_t message_type;
    size_t option_length = 1;
    if (!dhcp_get_option(packet->options,
                        length - sizeof(dhcp_packet_t),
                        DHCP_OPTION_MESSAGE_TYPE,
                        &message_type,
                        &option_length)) {
        return;
    }

    switch (message_type) {
        case DHCP_MESSAGE_OFFER:
            dhcp_state.offered_ip = packet->yiaddr;
            dhcp_state.server_ip = src_ip;

            // Get lease time
            option_length = sizeof(uint32_t);
            if (dhcp_get_option(packet->options,
                              length - sizeof(dhcp_packet_t),
                              DHCP_OPTION_LEASE_TIME,
                              &dhcp_state.lease_time,
                              &option_length)) {
                dhcp_state.lease_time = __builtin_bswap32(dhcp_state.lease_time);
            } else {
                dhcp_state.lease_time = 3600; // Default 1 hour
            }

            // Send REQUEST
            dhcp_send_request();
            break;

        case DHCP_MESSAGE_ACK:
            if (dhcp_state.callback) {
                dhcp_info_t info;
                info.ip_address = packet->yiaddr;
                info.server_ip = dhcp_state.server_ip;
                info.lease_time = dhcp_state.lease_time;

                // Get subnet mask
                option_length = sizeof(uint32_t);
                if (dhcp_get_option(packet->options,
                                  length - sizeof(dhcp_packet_t),
                                  DHCP_OPTION_SUBNET_MASK,
                                  &info.subnet_mask,
                                  &option_length)) {
                    // Got subnet mask
                }

                // Get router
                option_length = sizeof(uint32_t);
                if (dhcp_get_option(packet->options,
                                  length - sizeof(dhcp_packet_t),
                                  DHCP_OPTION_ROUTER,
                                  &info.router,
                                  &option_length)) {
                    // Got router
                }

                // Get DNS server
                option_length = sizeof(uint32_t);
                if (dhcp_get_option(packet->options,
                                  length - sizeof(dhcp_packet_t),
                                  DHCP_OPTION_DNS_SERVER,
                                  &info.dns_server,
                                  &option_length)) {
                    // Got DNS server
                }

                dhcp_state.callback(&info);
            }
            break;

        case DHCP_MESSAGE_NAK:
            // Start over
            dhcp_send_discover();
            break;
    }
}

bool dhcp_init(dhcp_callback_t callback) {
    if (dhcp_state.initialized) {
        return true;
    }

    if (!callback) {
        return false;
    }

    // Create UDP socket
    dhcp_state.socket = udp_create_socket(DHCP_CLIENT_PORT,
                                        dhcp_handle_packet);
    if (!dhcp_state.socket) {
        return false;
    }

    dhcp_state.callback = callback;
    dhcp_state.initialized = true;

    // Start DHCP process
    dhcp_send_discover();

    return true;
}

void dhcp_send_discover(void) {
    if (!dhcp_state.initialized) {
        return;
    }

    // Allocate packet buffer
    size_t packet_size = sizeof(dhcp_packet_t) + 32;
    dhcp_packet_t* packet = kmalloc(packet_size);
    if (!packet) {
        return;
    }

    // Initialize packet
    memset(packet, 0, packet_size);
    packet->op = DHCP_OP_REQUEST;
    packet->htype = 1; // Ethernet
    packet->hlen = 6; // MAC address length
    packet->xid = dhcp_state.xid = timer_get_ticks();
    packet->flags = 0x0000; // Unicast
    packet->magic_cookie = DHCP_MAGIC_COOKIE;

    // Set client hardware address
    memcpy(packet->chaddr, nic_get_mac_address(), 6);

    // Add options
    size_t options_offset = 0;
    uint8_t message_type = DHCP_MESSAGE_DISCOVER;
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_MESSAGE_TYPE, 1, &message_type);

    // Add parameter request list
    uint8_t params[] = {
        DHCP_OPTION_SUBNET_MASK,
        DHCP_OPTION_ROUTER,
        DHCP_OPTION_DNS_SERVER
    };
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_PARAMETER_REQUEST,
                    sizeof(params), params);

    // Add end option
    packet->options[options_offset++] = DHCP_OPTION_END;

    // Send packet
    udp_send(dhcp_state.socket, 0xFFFFFFFF, DHCP_SERVER_PORT,
             packet, sizeof(dhcp_packet_t) + options_offset);

    kfree(packet);
}

void dhcp_send_request(void) {
    if (!dhcp_state.initialized) {
        return;
    }

    // Allocate packet buffer
    size_t packet_size = sizeof(dhcp_packet_t) + 32;
    dhcp_packet_t* packet = kmalloc(packet_size);
    if (!packet) {
        return;
    }

    // Initialize packet
    memset(packet, 0, packet_size);
    packet->op = DHCP_OP_REQUEST;
    packet->htype = 1; // Ethernet
    packet->hlen = 6; // MAC address length
    packet->xid = dhcp_state.xid;
    packet->flags = 0x0000; // Unicast
    packet->magic_cookie = DHCP_MAGIC_COOKIE;

    // Set client hardware address
    memcpy(packet->chaddr, nic_get_mac_address(), 6);

    // Add options
    size_t options_offset = 0;
    uint8_t message_type = DHCP_MESSAGE_REQUEST;
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_MESSAGE_TYPE, 1, &message_type);

    // Add requested IP
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_REQUESTED_IP, 4, &dhcp_state.offered_ip);

    // Add server identifier
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_SERVER_ID, 4, &dhcp_state.server_ip);

    // Add parameter request list
    uint8_t params[] = {
        DHCP_OPTION_SUBNET_MASK,
        DHCP_OPTION_ROUTER,
        DHCP_OPTION_DNS_SERVER
    };
    dhcp_add_option(packet->options, &options_offset,
                    DHCP_OPTION_PARAMETER_REQUEST,
                    sizeof(params), params);

    // Add end option
    packet->options[options_offset++] = DHCP_OPTION_END;

    // Send packet
    udp_send(dhcp_state.socket, 0xFFFFFFFF, DHCP_SERVER_PORT,
             packet, sizeof(dhcp_packet_t) + options_offset);

    kfree(packet);
}

bool dhcp_is_initialized(void) {
    return dhcp_state.initialized;
}
