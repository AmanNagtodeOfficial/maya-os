#include "drivers/mouse.h"
#include "kernel/interrupts.h"

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

typedef struct {
    int x, y;
    uint8_t buttons;
    int x_velocity, y_velocity;
} mouse_state_t;

static mouse_state_t mouse_state = {160, 100, 0, 0, 0};

void mouse_init(void);
void mouse_handler(struct registers *regs);
mouse_state_t mouse_get_state(void);
void mouse_set_position(int x, int y);