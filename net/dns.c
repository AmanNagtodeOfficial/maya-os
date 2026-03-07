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

#define DNS_CACHE_SIZE 16

typedef struct {
    char domain[DNS_MAX_NAME_LENGTH];
    uint32_t ip;
    bool occupied;
} dns_cache_entry_t;

static struct {
    udp_socket_t* socket;
    uint32_t dns_server;
    uint16_t query_id;
    dns_callback_t callback;
    dns_cache_entry_t cache[DNS_CACHE_SIZE];
    bool initialized;
} dns_state;

static void dns_cache_insert(const char* domain, uint32_t ip) {
    for (int i = 0; i < DNS_CACHE_SIZE; i++) {
        if (!dns_state.cache[i].occupied) {
            strncpy(dns_state.cache[i].domain, domain, DNS_MAX_NAME_LENGTH - 1);
            dns_state.cache[i].ip = ip;
            dns_state.cache[i].occupied = true;
            return;
        }
    }
}

static uint32_t dns_cache_lookup(const char* domain) {
    for (int i = 0; i < DNS_CACHE_SIZE; i++) {
        if (dns_state.cache[i].occupied && strcmp(dns_state.cache[i].domain, domain) == 0) {
            return dns_state.cache[i].ip;
        }
    }
    return 0;
}

// ... (encode/decode functions)

bool dns_init(uint32_t dns_server) {
    if (dns_state.initialized) {
        return true;
    }

    // Create UDP socket
    dns_state.socket = udp_create_socket(0, dns_handle_response);
    if (!dns_state.socket) {
        return false;
    }

    dns_state.dns_server = dns_server;
    dns_state.query_id = 0;
    dns_state.callback = NULL;
    dns_state.initialized = true;

    return true;
}

bool dns_resolve(const char* domain, dns_callback_t callback) {
    if (!dns_state.initialized || !domain || !callback) {
        return false;
    }

    // Check cache
    uint32_t cached_ip = dns_cache_lookup(domain);
    if (cached_ip != 0) {
        callback(domain, cached_ip);
        return true;
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
    bool result = udp_send(dns_state.socket, dns_state.dns_server, DNS_PORT, 
                          query, offset);

    kfree(query);
    return result;
}


bool dns_is_initialized(void) {
    return dns_state.initialized;
}
