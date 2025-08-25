#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

void serial_init(uint16_t port);
void serial_putchar(uint16_t port, char c);
void serial_write(uint16_t port, const char *data, size_t size);
void serial_writestring(uint16_t port, const char *data);
char serial_getchar(uint16_t port);
uint8_t serial_received(uint16_t port);
uint8_t serial_is_transmit_empty(uint16_t port);

#endif
