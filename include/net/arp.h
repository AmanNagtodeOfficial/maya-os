#ifndef ARP_H
#define ARP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void arp_init(void);
void arp_resolve(uint32_t ip, uint8_t* mac);
void arp_handle_packet(const void* packet, size_t length);
void arp_send_request(uint32_t ip);

#endif
