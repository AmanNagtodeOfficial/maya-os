#ifndef MAYA_INPUT_H
#define MAYA_INPUT_H

#include <stdint.h>

// Input Event Types
typedef enum {
 INPUT_KEY_PRESS,
    INPUT_KEY_RELEASE,
 INPUT_MOUSE_MOVE,
 INPUT_MOUSE_CLICK,
 INPUT_MOUSE_RELEASE
} input_event_type_t;

// Input Event Structure
typedef struct {
    input_event_type_t type;
    union {
 struct {
            char ascii;
 uint8_t scancode;
 uint8_t modifiers; // Ctrl, Alt, Shift flags
 } key;
 struct {
 int x, y;
 uint8_t buttons; // Left, Right, Middle buttons
 } mouse;
    } data;
 uint32_t timestamp;
} maya_input_event_t;

// Input Manager Structure
typedef struct {
    maya_input_event_t event_queue[32];
 int queue_head;
 int queue_tail;
 int queue_count;

    // Current state
    int mouse_x;
    int mouse_y;
    uint8_t mouse_buttons;
    uint8_t key_modifiers;

    // Tablet/Touch State
    uint8_t tablet_mode_active;
    uint8_t is_touch_event;

    // Double click tracking
    uint32_t last_click_time;
    int last_click_x;
    int last_click_y;
    uint8_t is_double_click;

    // Key repeat
 char last_key;
 uint32_t key_repeat_start;
 uint32_t key_repeat_delay;
} maya_input_manager_t;

// Function Declarations
void maya_input_init(void);
void maya_input_update(void);
uint8_t maya_input_has_events(void);
maya_input_event_t maya_input_get_event(void);
void maya_input_add_key_event(char ascii, uint8_t scancode, uint8_t pressed);
void maya_input_add_mouse_event(int x, int y, uint8_t buttons);

// Callbacks for drivers
void maya_input_add_key_event_callback(char ascii);
void maya_input_add_mouse_event_callback(int x, int y, uint8_t buttons);
void maya_input_add_touch_event_callback(int x, int y, uint8_t is_down);

// Tablet / Touch APIs
void maya_input_set_tablet_mode(uint8_t active);
uint8_t maya_input_is_tablet_mode(void);

// Keyboard state queries
uint8_t maya_input_is_key_pressed(uint8_t scancode);
uint8_t maya_input_is_ctrl_pressed(void);
uint8_t maya_input_is_alt_pressed(void);
uint8_t maya_input_is_shift_pressed(void);

// Mouse state queries
void maya_input_get_mouse_pos(int* x, int* y);
uint8_t maya_input_is_mouse_button_pressed(uint8_t button);
uint8_t maya_input_is_double_click(void);
void maya_input_clear_double_click(void);

// Input focus management
void maya_input_set_focus(uint32_t window_id);
uint32_t maya_input_get_focus(void);

#define MAYA_KEY_CTRL 29
#define MAYA_KEY_ALT 56
#define MAYA_KEY_SHIFT 42
#define MAYA_KEY_SUPER 91 // Left Windows Key (Standard Scancode)
#define KEY_ESC 1
#define KEY_ENTER 28
#define KEY_SPACE 57
#define KEY_TAB 15
#define KEY_BACKSPACE 14
#define KEY_DELETE 83
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_F4 62

// Mouse button constants
#define MAYA_MOUSE_LEFT 1
#define MAYA_MOUSE_RIGHT 2
#define MAYA_MOUSE_MIDDLE 4

#endif
