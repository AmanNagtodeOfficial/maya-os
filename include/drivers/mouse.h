#ifndef DRIVER_MOUSE_H
#define DRIVER_MOUSE_H

#include <stdint.h>
#include "kernel/interrupts.h"

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

typedef struct {
    int x, y;
    uint8_t buttons;
    int x_velocity, y_velocity;
    void (*callback)(int x, int y, uint8_t buttons);
} mouse_state_t;

void mouse_init(void);
mouse_state_t mouse_get_state(void);
void mouse_set_position(int x, int y);
typedef void (*mouse_callback_t)(int x, int y, uint8_t buttons);
void mouse_set_callback(mouse_callback_t callback);

#endif