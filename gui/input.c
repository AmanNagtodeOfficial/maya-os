#include "gui/input.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "kernel/timer.h"
#include "libc/string.h"
#include "libc/stdio.h"

static maya_input_manager_t input_manager;

void maya_input_init(void) {
    memset(&input_manager, 0, sizeof(maya_input_manager_t));
    input_manager.queue_head = 0;
    input_manager.queue_tail = 0;
    input_manager.queue_count = 0;
    input_manager.key_repeat_delay = 500; // ms

    // Registration with drivers
    keyboard_set_callback(maya_input_add_key_event_callback);
    mouse_set_callback(maya_input_add_mouse_event_callback);

    printf("Maya Input Manager initialized\n");
}

void maya_input_add_key_event_callback(char ascii) {
    // This is called by the keyboard driver
    // For now, assume it's a press event
    maya_input_add_key_event(ascii, 0, 1);
}

void maya_input_add_mouse_event_callback(int x, int y, uint8_t buttons) {
    // This is called by the mouse driver
    input_manager.is_touch_event = 0; // Standard mouse event
    maya_input_add_mouse_event(x, y, buttons);
}

void maya_input_add_touch_event_callback(int x, int y, uint8_t is_down) {
    // This simulates a touch driver (capacitive or resistive)
    input_manager.is_touch_event = 1;

    // We translate touch events directly to mouse events for GUI simplicity,
    // but the GUI could query `input_manager.is_touch_event` if it needs to care.
    uint8_t simulated_btn = is_down ? MAYA_MOUSE_LEFT : 0;
    maya_input_add_mouse_event(x, y, simulated_btn);
}

void maya_input_add_key_event(char ascii, uint8_t scancode, uint8_t pressed) {
    if (input_manager.queue_count >= 32) return;

    // Maintain global modifier state
    if (scancode == MAYA_KEY_CTRL) {
        if (pressed) input_manager.key_modifiers |= 0x01; // Ctrl bit
        else input_manager.key_modifiers &= ~0x01;
    } else if (scancode == MAYA_KEY_ALT) {
        if (pressed) input_manager.key_modifiers |= 0x02; // Alt bit
        else input_manager.key_modifiers &= ~0x02;
    } else if (scancode == MAYA_KEY_SHIFT) {
        if (pressed) input_manager.key_modifiers |= 0x04; // Shift bit
        else input_manager.key_modifiers &= ~0x04;
    }

    // Global Shortcuts Trigger
    if (pressed) {
        // [Super] -> Toggle Windows Start Menu; [Super+Tab] -> Task View
        if (scancode == MAYA_KEY_SUPER) {
            if (input_manager.key_modifiers & 0x08) { // Super+Tab not easy without two-key tracking
                extern void taskview_open(uint8_t);
                taskview_open(0); // Full Win+Tab task view
            } else {
                extern void desktop_toggle_start_menu(void);
                desktop_toggle_start_menu();
            }
            return; // Don't propagate global OS key
        }
        // [Alt + F4] -> Close Application
        if ((input_manager.key_modifiers & 0x02) && scancode == KEY_F4) {
            extern void window_destroy(void* window);
            extern void* window_get_focused(void);
            void* focused = window_get_focused();
            if (focused) window_destroy(focused);
            return; // Handled
        }
        // Basic [Ctrl+C] [Ctrl+V] hooks for future
        if ((input_manager.key_modifiers & 0x01) && ascii == 'c') {
            printf("Global Copy Triggered\n");
            return;
        }
        if ((input_manager.key_modifiers & 0x01) && ascii == 'v') {
            printf("Global Paste Triggered\n");
            return;
        }
        // [Alt+Tab] -> Compact task switcher
        if ((input_manager.key_modifiers & 0x02) && scancode == KEY_TAB) {
            extern void taskview_open(uint8_t);
            taskview_open(1); // Alt+Tab compact mode
            return;
        }
    }

    maya_input_event_t* event = &input_manager.event_queue[input_manager.queue_head];
    event->type = pressed ? INPUT_KEY_PRESS : INPUT_KEY_RELEASE;
    event->data.key.ascii = ascii;
    event->data.key.scancode = scancode;
    event->data.key.modifiers = input_manager.key_modifiers;
    event->timestamp = timer_get_ticks();

    input_manager.queue_head = (input_manager.queue_head + 1) % 32;
    input_manager.queue_count++;
}

void maya_input_add_mouse_event(int x, int y, uint8_t buttons) {
    if (input_manager.queue_count >= 32) return;

    maya_input_event_t* event = &input_manager.event_queue[input_manager.queue_head];
    
    if (x != input_manager.mouse_x || y != input_manager.mouse_y) {
        event->type = INPUT_MOUSE_MOVE;
    } else if (buttons != input_manager.mouse_buttons) {
        event->type = (buttons > input_manager.mouse_buttons) ? INPUT_MOUSE_CLICK : INPUT_MOUSE_RELEASE;
        
        // Double click detection
        if (event->type == INPUT_MOUSE_CLICK && buttons == MAYA_MOUSE_LEFT) {
            uint32_t now = timer_get_ticks();
            if (now - input_manager.last_click_time < 50 && // ~500ms assuming 100hz timer
                abs(x - input_manager.last_click_x) < 5 &&
                abs(y - input_manager.last_click_y) < 5) {
                input_manager.is_double_click = 1;
            } else {
                input_manager.is_double_click = 0;
            }
            input_manager.last_click_time = now;
            input_manager.last_click_x = x;
            input_manager.last_click_y = y;
        }
    } else {
        return; // No change
    }

    event->data.mouse.x = x;
    event->data.mouse.y = y;
    event->data.mouse.buttons = buttons;
    event->timestamp = timer_get_ticks();

    input_manager.mouse_x = x;
    input_manager.mouse_y = y;
    input_manager.mouse_buttons = buttons;

    input_manager.queue_head = (input_manager.queue_head + 1) % 32;
    input_manager.queue_count++;
}

// Keyboard state queries
uint8_t maya_input_is_key_pressed(uint8_t scancode) {
    // Requires physical driver array buffer, simplified for now
    return 0;
}
uint8_t maya_input_is_ctrl_pressed(void) { return (input_manager.key_modifiers & 0x01) != 0; }
uint8_t maya_input_is_alt_pressed(void) { return (input_manager.key_modifiers & 0x02) != 0; }
uint8_t maya_input_is_shift_pressed(void) { return (input_manager.key_modifiers & 0x04) != 0; }

uint8_t maya_input_has_events(void) {
    return input_manager.queue_count > 0;
}

maya_input_event_t maya_input_get_event(void) {
    maya_input_event_t event = {0};
    if (input_manager.queue_count > 0) {
        event = input_manager.event_queue[input_manager.queue_tail];
        input_manager.queue_tail = (input_manager.queue_tail + 1) % 32;
        input_manager.queue_count--;
    }
    return event;
}

void maya_input_update(void) {
    // Handle key repeats or other periodic input tasks
}

void maya_input_get_mouse_pos(int* x, int* y) {
    if (x) *x = input_manager.mouse_x;
    if (y) *y = input_manager.mouse_y;
}

uint8_t maya_input_is_mouse_button_pressed(uint8_t button) {
    return (input_manager.mouse_buttons & button) != 0;
}

uint8_t maya_input_is_double_click(void) {
    return input_manager.is_double_click;
}

void maya_input_clear_double_click(void) {
    input_manager.is_double_click = 0;
}

void maya_input_set_tablet_mode(uint8_t active) {
    input_manager.tablet_mode_active = active;
}

uint8_t maya_input_is_tablet_mode(void) {
    return input_manager.tablet_mode_active;
}
