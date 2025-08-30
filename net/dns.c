/**
 * Maya OS DNS Client Implementation
 * Updated: 2025-08-29 11:13:08 UTC
 * Author: AmanNagtodeOfficial
 */

#include "net/dns.h"
#include "net/udp.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define DNS_PORT 53
#define DNS_MAX_PACKET_SIZE 512
#define DNS_MAX_NAME_LENGTH 256
#define DNS_TIMEOUT 5000 // milliseconds

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t questions;
    uint16_t answers;
    uint16_t authority;
    uint16_t additional;
} __attribute__((packed)) dns_header_t;

typedef struct {
    uint16_t type;
    uint16_t class;
} __attribute__((packed)) dns_question_t;

typedef struct {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t length;
} __attribute__((packed)) dns_record_t;

static struct {
    udp_socket_t* socket;
    uint16_t query_id;
    dns_callback_t callback;
    bool initialized;
} dns_state;

static void dns_encode_name(const char* domain, uint8_t* buffer, size_t* offset) {
    const char* segment = domain;
    uint8_t* length_ptr = buffer + *offset;
    (*offset)++;

    while (*segment) {
        if (*segment == '.') {
            length_ptr = buffer + *offset;
            (*offset)++;
            segment++;
            continue;
        }

        buffer[*offset] = *segment;
        (*offset)++;
        (*length_ptr)++;
        segment++;
    }

    buffer[*offset] = 0;
    (*offset)++;
}

static bool dns_decode_name(const uint8_t* packet, size_t packet_size,
                          size_t* offset, char* name, size_t name_size) {
    size_t name_offset = 0;
    bool first = true;

    while (*offset < packet_size) {
        uint8_t length = packet[*offset];

        // Check for compression pointer
        if (length & 0xC0) {
            if (*offset + 1 >= packet_size) {
                return false;
            }

            uint16_t pointer = ((length & 0x3F) << 8) | packet[*offset + 1];
            size_t saved_offset = *offset;
            *offset = pointer;

            if (!dns_decode_name(packet, packet_size, offset, name, name_size)) {
                return false;
            }

            *offset = saved_offset + 2;
            return true;
        }

        (*offset)++;

        if (length == 0) {
            break;
        }

        if (*offset + length > packet_size) {
            return false;
        }

        if (!first && name_offset < name_size - 1) {
            name[name_offset++] = '.';
        }
        first = false;

        for (uint8_t i = 0; i < length && name_offset < name_size - 1; i++) {
            name[name_offset++] = packet[(*offset)++];
        }
    }

    if (name_offset < name_size) {
        name[name_offset] = '\0';
    }

    return true;
}

static void dns_handle_response(uint32_t src_ip, uint16_t src_port,
                              const void* data, size_t length) {
    if (length < sizeof(dns_header_t)) {
        return;
    }

    const dns_header_t* header = (const dns_header_t*)data;
    if ((header->flags & 0x8000) == 0) {
        return; // Not a response
    }

    size_t offset = sizeof(dns_header_t);
    char name[DNS_MAX_NAME_LENGTH];
    uint32_t ip_address = 0;

    // Skip questions
    uint16_t questions = (header->questions >> 8) | (header->questions << 8);
    for (uint16_t i = 0; i < questions; i++) {
        if (!dns_decode_name(data, length, &offset, name, sizeof(name))) {
            return;
        }
        offset += sizeof(dns_question_t);
    }

    // Process answers
    uint16_t answers = (header->answers >> 8) | (header->answers << 8);
    for (uint16_t i = 0; i < answers; i++) {
        if (!dns_decode_name(data, length, &offset, name, sizeof(name))) {
            return;
        }

        if (offset + sizeof(dns_record_t) > length) {
            return;
        }

        const dns_record_t* record = (const dns_record_t*)((uint8_t*)data + offset);
        offset += sizeof(dns_record_t);

        uint16_t type = (record->type >> 8) | (record->type << 8);
        uint16_t rec_length = (record->length >> 8) | (record->length << 8);

        if (type == 1 && rec_length == 4) { // A record
            if (offset + 4 > length) {
                return;
            }
            memcpy(&ip_address, (uint8_t*)data + offset, 4);
            break;
        }

        offset += rec_length;
    }

    if (dns_state.callback) {
        dns_state.callback(name, ip_address);
    }
}

bool dns_init(uint32_t dns_server) {
    if (dns_state.initialized) {
        return true;
    }

    // Create UDP socket
    dns_state.socket = udp_create_socket(0, dns_handle_response);
    if (!dns_state.socket) {
        return false;
    }

    dns_state.query_id = 0;
    dns_state.callback = NULL;
    dns_state.initialized = true;

    return true;
}

bool dns_resolve(const char* domain, dns_callback_t callback) {
    if (!dns_state.initialized || !domain || !callback) {
        return false;
    }

    // Allocate query buffer
    uint8_t* query = kmalloc(DNS_MAX_PACKET_SIZE);
    if (!query) {
        return false;
    }

    // Build DNS query
    dns_header_t* header = (dns_header_t*)query;
    header->id = ++dns_state.query_id;
    header->flags = 0x0100; // Standard query
    header->questions = 0x0100; // One question
    header->answers = 0;
    header->authority = 0;
    header->additional = 0;

    size_t offset = sizeof(dns_header_t);
    dns_encode_name(domain, query, &offset);

    // Add question
    dns_question_t* question = (dns_question_t*)(query + offset);
    question->type = 0x0100; // A record
    question->class = 0x0100; // IN class
    offset += sizeof(dns_question_t);

    dns_state.callback = callback;

    // Send query
    bool result = udp_send(dns_state.socket, dns_server, DNS_PORT, 
                          query, offset);

    kfree(query);
    return result;
}

bool dns_is_initialized(void) {
    return dns_state.initialized;
}
