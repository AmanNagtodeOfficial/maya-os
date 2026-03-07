#ifndef NET_DHCP_H
#define NET_DHCP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t router;
    uint32_t dns_server;
    uint32_t server_ip;
    uint32_t lease_time;
} dhcp_info_t;

typedef void (*dhcp_callback_t)(const dhcp_info_t* info);

bool dhcp_init(dhcp_callback_t callback);
void dhcp_send_discover(void);
void dhcp_send_request(void);
bool dhcp_is_initialized(void);

#endif
