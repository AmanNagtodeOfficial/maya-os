#ifndef NET_DNS_H
#define NET_DNS_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*dns_callback_t)(const char* name, uint32_t ip);

bool dns_init(uint32_t dns_server);
bool dns_resolve(const char* domain, dns_callback_t callback);
bool dns_is_initialized(void);

#endif
