/**
 * Maya OS Serial Port Driver
 * Updated: 2025-08-29 11:02:38 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/serial.h"
#include "kernel/io.h"

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

#define SERIAL_LINE_ENABLE_DLAB         0x80
#define SERIAL_FIFO_ENABLE             0xC7
#define SERIAL_STOP_ONE_BIT            0x03
#define SERIAL_EIGHT_BITS              0x03

static bool ports_initialized[4] = {false, false, false, false};

static bool serial_configure_baud_rate(uint16_t port, uint16_t divisor) {
    outb(SERIAL_LINE_COMMAND_PORT(port), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(port), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(port), divisor & 0x00FF);
    return true;
}

static bool serial_configure_line(uint16_t port) {
    // Configure 8 bits, no parity, one stop bit
    outb(SERIAL_LINE_COMMAND_PORT(port), SERIAL_EIGHT_BITS);
    return true;
}

static bool serial_configure_fifo(uint16_t port) {
    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_FIFO_COMMAND_PORT(port), SERIAL_FIFO_ENABLE);
    return true;
}

static bool serial_configure_modem(uint16_t port) {
    // IRQs enabled, RTS/DSR set
    outb(SERIAL_MODEM_COMMAND_PORT(port), 0x0B);
    return true;
}

bool serial_init(uint16_t port) {
    int port_index;
    switch (port) {
        case COM1: port_index = 0; break;
        case COM2: port_index = 1; break;
        case COM3: port_index = 2; break;
        case COM4: port_index = 3; break;
        default: return false;
    }

    if (ports_initialized[port_index]) {
        return true;
    }

    // Set baud rate to 38400
    if (!serial_configure_baud_rate(port, 3)) {
        return false;
    }

    if (!serial_configure_line(port)) {
        return false;
    }

    if (!serial_configure_fifo(port)) {
        return false;
    }

    if (!serial_configure_modem(port)) {
        return false;
    }

    ports_initialized[port_index] = true;
    return true;
}

bool serial_is_transmit_empty(uint16_t port) {
    return inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20;
}

bool serial_write_byte(uint16_t port, uint8_t byte) {
    int port_index;
    switch (port) {
        case COM1: port_index = 0; break;
        case COM2: port_index = 1; break;
        case COM3: port_index = 2; break;
        case COM4: port_index = 3; break;
        default: return false;
    }

    if (!ports_initialized[port_index]) {
        return false;
    }

    while (!serial_is_transmit_empty(port)) {
        // Wait for empty transmit buffer
    }

    outb(port, byte);
    return true;
}

size_t serial_write(uint16_t port, const void* data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }

    const uint8_t* bytes = (const uint8_t*)data;
    size_t written = 0;

    for (size_t i = 0; i < size; i++) {
        if (!serial_write_byte(port, bytes[i])) {
            break;
        }
        written++;
    }

    return written;
}

bool serial_is_initialized(uint16_t port) {
    switch (port) {
        case COM1: return ports_initialized[0];
        case COM2: return ports_initialized[1];
        case COM3: return ports_initialized[2];
        case COM4: return ports_initialized[3];
        default: return false;
    }
}
