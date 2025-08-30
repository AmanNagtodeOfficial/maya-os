/**
 * Maya OS Mouse Driver
 * Updated: 2025-08-29 10:57:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/mouse.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
#include "gui/graphics.h"

#define MOUSE_PORT 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_COMMAND 0x64
#define MOUSE_ACK 0xFA

static mouse_state_t mouse_state;
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
static bool mouse_initialized = false;

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & 1) == 1) {
                return;
            }
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & 2) == 0) {
                return;
            }
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_COMMAND, 0xD4);
    mouse_wait(1);
    outb(MOUSE_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_PORT);
}

static void mouse_handler(struct regs* r) {
    uint8_t status = inb(MOUSE_STATUS);
    if (!(status & 0x20)) {
        return;
    }

    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_read();
            if (!(mouse_byte[0] & 0x08)) {
                return;
            }
            mouse_cycle++;
            break;

        case 1:
            mouse_byte[1] = mouse_read();
            mouse_cycle++;
            break;

        case 2:
            mouse_byte[2] = mouse_read();
            
            // Update mouse state
            mouse_state.buttons = mouse_byte[0] & 0x07;
            
            // Calculate movement
            int8_t x_mov = mouse_byte[1];
            int8_t y_mov = mouse_byte[2];
            
            if (mouse_byte[0] & 0x10) x_mov |= 0xFFFFFF00;
            if (mouse_byte[0] & 0x20) y_mov |= 0xFFFFFF00;
            
            // Update position with bounds checking
            mouse_state.x += x_mov;
            mouse_state.y -= y_mov;  // Y is inverted
            
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (mouse_state.x >= SCREEN_WIDTH) mouse_state.x = SCREEN_WIDTH - 1;
            if (mouse_state.y >= SCREEN_HEIGHT) mouse_state.y = SCREEN_HEIGHT - 1;
            
            // Call registered callback if any
            if (mouse_state.callback) {
                mouse_state.callback(mouse_state.x, mouse_state.y, mouse_state.buttons);
            }
            
            mouse_cycle = 0;
            break;
    }
}

bool mouse_init(void) {
    if (mouse_initialized) {
        return true;
    }

    // Initialize mouse state
    memset(&mouse_state, 0, sizeof(mouse_state_t));
    mouse_state.x = SCREEN_WIDTH / 2;
    mouse_state.y = SCREEN_HEIGHT / 2;

    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(MOUSE_COMMAND, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_COMMAND, 0x20);
    mouse_wait(0);
    uint8_t status = inb(MOUSE_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_COMMAND, 0x60);
    mouse_wait(1);
    outb(MOUSE_PORT, status);

    // Set default settings
    mouse_write(0xF6);
    mouse_read();  // ACK

    // Enable mouse
    mouse_write(0xF4);
    mouse_read();  // ACK

    // Install mouse handler
    irq_install_handler(12, mouse_handler);

    mouse_initialized = true;
    return true;
}

void mouse_set_callback(mouse_callback_t callback) {
    if (!mouse_initialized) {
        return;
    }
    mouse_state.callback = callback;
}

mouse_state_t mouse_get_state(void) {
    return mouse_state;
}

bool mouse_is_initialized(void) {
    return mouse_initialized;
}
