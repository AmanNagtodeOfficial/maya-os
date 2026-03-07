#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/input.h"
#include "libc/string.h"

// Basic Virtual Keyboard implementation
static window_t* vk_window = NULL;

// Simplified single row of keys for demonstration
static char* keys[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", 
                       "A", "S", "D", "F", "G", "H", "J", "K", "L", 
                       "Z", "X", "C", "V", "B", "N", "M", "SPACE", "ENTER"};
static int num_keys = 28;

typedef struct {
    int x, y, w, h;
    char* label;
    int scancode; // Simplified: usually would map label to exact scancode
} vk_key_t;

static vk_key_t vk_layout[28];

void virtual_keyboard_draw_content(void) {
    if (!vk_window || !vk_window->visible) return;

    // Background
    window_draw_rect(vk_window, 0, 0, vk_window->width - 4, vk_window->height - WINDOW_TITLE_HEIGHT - 4, 0x333333);

    // Draw Keys
    for (int i = 0; i < num_keys; i++) {
        vk_key_t* k = &vk_layout[i];
        graphics_fill_rect(vk_window->x + k->x, vk_window->y + WINDOW_TITLE_HEIGHT + k->y, k->w, k->h, 0x555555);
        graphics_draw_rect(vk_window->x + k->x, vk_window->y + WINDOW_TITLE_HEIGHT + k->y, k->w, k->h, 0x222222);
        
        // Center text roughly
        graphics_draw_text(k->label, vk_window->x + k->x + (k->w/2) - 4, vk_window->y + WINDOW_TITLE_HEIGHT + k->y + (k->h/2) - 6, 0xFFFFFF);
    }

    window_refresh(vk_window);
}

void virtual_keyboard_handle_mouse(int x, int y, uint8_t buttons) {
    if (!vk_window || !vk_window->visible) return;
    
    // Convert absolute x/y to window relative
    int rel_x = x - vk_window->x;
    int rel_y = y - (vk_window->y + WINDOW_TITLE_HEIGHT);

    if (buttons & MAYA_MOUSE_LEFT) {
        for (int i = 0; i < num_keys; i++) {
            vk_key_t* k = &vk_layout[i];
            if (rel_x >= k->x && rel_x <= k->x + k->w &&
                rel_y >= k->y && rel_y <= k->y + k->h) {
                
                // Simulate key press into system queue
                char ascii = k->label[0];
                if (strcmp(k->label, "SPACE") == 0) ascii = ' ';
                if (strcmp(k->label, "ENTER") == 0) ascii = '\n';

                maya_input_add_key_event(ascii, 0, 1); // Mock scancode 0 for now
                // Optional: add visual click feedback by redrawing key highlighted
                break;
            }
        }
    }
}

void virtual_keyboard_init(void) {
    vk_window = window_create("Virtual Keyboard", 100, 300, 600, 150);
    if (!vk_window) return;

    // Initialize layout positions
    int start_x = 10, start_y = 10;
    int key_w = 40, key_h = 30;

    for (int i = 0; i < num_keys; i++) {
        vk_layout[i].label = keys[i];
        
        if (strcmp(keys[i], "SPACE") == 0) {
            start_x = 150; start_y += 40;
            key_w = 200;
        } else if (strcmp(keys[i], "ENTER") == 0) {
            start_x += key_w + 10;
            key_w = 80;
        } else if (i == 10 || i == 19) { // New rows roughly
            start_x = 25 + (i == 19 ? 15 : 0);
            start_y += 40;
            key_w = 40;
        }
        
        vk_layout[i].x = start_x;
        vk_layout[i].y = start_y;
        vk_layout[i].w = key_w;
        vk_layout[i].h = key_h;
        
        start_x += key_w + 5;
    }

    virtual_keyboard_draw_content();
}

void virtual_keyboard_draw(void) {
    if (vk_window && vk_window->visible) {
        window_render(vk_window);
    }
}
