#include "net/ethernet.h"

#define ETHERNET_TYPE_ARP  0x0806
#define ETHERNET_TYPE_IP   0x0800

typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6]; 
    uint16_t type;
} __attribute__((packed)) ethernet_header_t;

void ethernet_init(void);
void ethernet_send_packet(uint8_t *dest_mac, uint16_t type, uint8_t *data, uint32_t length);
void ethernet_receive_handler(uint8_t *packet, uint32_t length);