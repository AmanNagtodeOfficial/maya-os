/**
 * Maya OS Keyboard Driver
 * Updated: 2025-08-29 11:10:30 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/keyboard.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64

#define KEYBOARD_BUFFER_SIZE 256
#define KEY_RELEASE_BIT 0x80

// US QWERTY keyboard layout scancode mapping
static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '',
    '	', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '
',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ''', '`',
    0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '',
    '	', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '
',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static struct {
    char buffer[KEYBOARD_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    bool shift_pressed;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
    keyboard_callback_t callback;
    bool initialized;
} keyboard_state;

static void keyboard_buffer_push(char c) {
    uint32_t next = (keyboard_state.head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != keyboard_state.tail) {
        keyboard_state.buffer[keyboard_state.head] = c;
        keyboard_state.head = next;
    }
}

static char keyboard_buffer_pop(void) {
    if (keyboard_state.tail != keyboard_state.head) {
        char c = keyboard_state.buffer[keyboard_state.tail];
        keyboard_state.tail = (keyboard_state.tail + 1) % KEYBOARD_BUFFER_SIZE;
        return c;
    }
    return 0;
}

static void keyboard_handler(struct regs* r) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    bool released = scancode & KEY_RELEASE_BIT;
    scancode &= ~KEY_RELEASE_BIT;

    // Handle special keys
    switch (scancode) {
        case 0x2A: // Left shift
        case 0x36: // Right shift
            keyboard_state.shift_pressed = !released;
            break;

        case 0x3A: // Caps Lock
            if (!released) {
                keyboard_state.caps_lock = !keyboard_state.caps_lock;
            }
            break;

        case 0x45: // Num Lock
            if (!released) {
                keyboard_state.num_lock = !keyboard_state.num_lock;
            }
            break;

        case 0x46: // Scroll Lock
            if (!released) {
                keyboard_state.scroll_lock = !keyboard_state.scroll_lock;
            }
            break;

        default:
            if (!released && scancode < sizeof(scancode_to_ascii)) {
                char c;
                if (keyboard_state.shift_pressed) {
                    c = scancode_to_ascii_shift[scancode];
                } else {
                    c = scancode_to_ascii[scancode];
                }

                if (c) {
                    // Handle caps lock
                    if (keyboard_state.caps_lock && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                        c ^= 0x20;  // Toggle case
                    }

                    keyboard_buffer_push(c);

                    // Call callback if registered
                    if (keyboard_state.callback) {
                        keyboard_state.callback(c);
                    }
                }
            }
            break;
    }
}

bool keyboard_init(void) {
    if (keyboard_state.initialized) {
        return true;
    }

    // Initialize state
    memset(&keyboard_state, 0, sizeof(keyboard_state));

    // Install interrupt handler
    irq_install_handler(1, keyboard_handler);

    // Enable keyboard
    while ((inb(KEYBOARD_STATUS_PORT) & 2) != 0);
    outb(KEYBOARD_COMMAND_PORT, 0xAE);

    // Reset keyboard
    outb(KEYBOARD_DATA_PORT, 0xFF);
    while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
    if (inb(KEYBOARD_DATA_PORT) != 0xFA) {
        return false;
    }

    keyboard_state.initialized = true;
    return true;
}

void keyboard_set_callback(keyboard_callback_t callback) {
    if (!keyboard_state.initialized) {
        return;
    }
    keyboard_state.callback = callback;
}

char keyboard_get_char(void) {
    if (!keyboard_state.initialized) {
        return 0;
    }
    return keyboard_buffer_pop();
}

size_t keyboard_read(char* buffer, size_t size) {
    if (!keyboard_state.initialized || !buffer || size == 0) {
        return 0;
    }

    size_t read = 0;
    char c;

    while (read < size && (c = keyboard_buffer_pop())) {
        buffer[read++] = c;
    }

    return read;
}

bool keyboard_is_shift_pressed(void) {
    return keyboard_state.shift_pressed;
}

bool keyboard_is_caps_lock_on(void) {
    return keyboard_state.caps_lock;
}

bool keyboard_is_num_lock_on(void) {
    return keyboard_state.num_lock;
}

bool keyboard_is_scroll_lock_on(void) {
    return keyboard_state.scroll_lock;
}

bool keyboard_is_initialized(void) {
    return keyboard_state.initialized;
}

void keyboard_cleanup(void) {
    if (!keyboard_state.initialized) {
        return;
    }

    // Disable keyboard
    while ((inb(KEYBOARD_STATUS_PORT) & 2) != 0);
    outb(KEYBOARD_COMMAND_PORT, 0xAD);

    // Uninstall interrupt handler
    irq_install_handler(1, NULL);

    keyboard_state.initialized = false;
}
