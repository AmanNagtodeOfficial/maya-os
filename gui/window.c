/**
 * Maya OS Window Manager
 * Updated: 2025-08-29 10:57:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "gui/window.h"
#include "gui/graphics.h"
#include "kernel/memory.h"
#include "drivers/mouse.h"
#include "libc/string.h"

#define MAX_WINDOWS 32
#define WINDOW_TITLE_HEIGHT 20
#define WINDOW_BORDER_WIDTH 2
#define MIN_WINDOW_WIDTH 100
#define MIN_WINDOW_HEIGHT 50

typedef struct {
    window_t* windows[MAX_WINDOWS];
    window_t* focused_window;
    uint32_t window_count;
    bool initialized;
} window_manager_t;

static window_manager_t wm;

bool window_manager_init(void) {
    if (wm.initialized) {
        return true;
    }

    memset(&wm, 0, sizeof(window_manager_t));
    wm.initialized = true;
    return true;
}

window_t* window_create(const char* title, int x, int y, int width, int height) {
    if (!wm.initialized || !title || width < MIN_WINDOW_WIDTH || height < MIN_WINDOW_HEIGHT) {
        return NULL;
    }

    if (wm.window_count >= MAX_WINDOWS) {
        return NULL;
    }

    // Allocate window structure
    window_t* window = kmalloc(sizeof(window_t));
    if (!window) {
        return NULL;
    }

    // Initialize window properties
    strncpy(window->title, title, WINDOW_TITLE_MAX - 1);
    window->title[WINDOW_TITLE_MAX - 1] = '\0';
    
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->visible = true;
    window->dragging = false;
    window->resizing = false;
    window->id = wm.window_count++;

    // Create window buffer
    window->buffer = kmalloc(width * height * sizeof(uint32_t));
    if (!window->buffer) {
        kfree(window);
        wm.window_count--;
        return NULL;
    }

    // Clear window buffer
    memset(window->buffer, 0, width * height * sizeof(uint32_t));

    // Add to window list
    wm.windows[window->id] = window;
    window_focus(window);

    return window;
}

void window_destroy(window_t* window) {
    if (!wm.initialized || !window) {
        return;
    }

    // Remove from window list
    for (uint32_t i = 0; i < wm.window_count; i++) {
        if (wm.windows[i] == window) {
            // Shift remaining windows
            for (uint32_t j = i; j < wm.window_count - 1; j++) {
                wm.windows[j] = wm.windows[j + 1];
                wm.windows[j]->id = j;
            }
            wm.window_count--;
            break;
        }
    }

    // Update focused window
    if (wm.focused_window == window) {
        wm.focused_window = (wm.window_count > 0) ? wm.windows[wm.window_count - 1] : NULL;
    }

    // Free resources
    if (window->buffer) {
        kfree(window->buffer);
    }
    kfree(window);
}

void window_render(window_t* window) {
    if (!wm.initialized || !window || !window->visible) {
        return;
    }

    // Draw window border
    graphics_draw_rect(window->x, window->y,
                      window->width, window->height,
                      (window == wm.focused_window) ? WINDOW_ACTIVE_BORDER : WINDOW_INACTIVE_BORDER);

    // Draw title bar
    graphics_fill_rect(window->x + WINDOW_BORDER_WIDTH,
                      window->y + WINDOW_BORDER_WIDTH,
                      window->width - (2 * WINDOW_BORDER_WIDTH),
                      WINDOW_TITLE_HEIGHT,
                      (window == wm.focused_window) ? WINDOW_ACTIVE_TITLE : WINDOW_INACTIVE_TITLE);

    // Draw title text
    graphics_draw_text(window->title,
                      window->x + 5,
                      window->y + (WINDOW_TITLE_HEIGHT / 2) - 4,
                      WINDOW_TEXT_COLOR);

    // Draw window contents
    graphics_blit(window->buffer,
                 window->x + WINDOW_BORDER_WIDTH,
                 window->y + WINDOW_TITLE_HEIGHT + WINDOW_BORDER_WIDTH,
                 window->width - (2 * WINDOW_BORDER_WIDTH),
                 window->height - WINDOW_TITLE_HEIGHT - (2 * WINDOW_BORDER_WIDTH));
}

void window_render_all(void) {
    if (!wm.initialized) {
        return;
    }

    // Clear screen
    graphics_clear(DESKTOP_BACKGROUND);

    // Render all windows bottom to top
    for (uint32_t i = 0; i < wm.window_count; i++) {
        window_render(wm.windows[i]);
    }

    // Update screen
    graphics_update();
}

bool window_handle_mouse(int x, int y, uint8_t buttons) {
    if (!wm.initialized) {
        return false;
    }

    static int drag_start_x = 0;
    static int drag_start_y = 0;

    // Check for window interactions (top to bottom)
    for (int i = wm.window_count - 1; i >= 0; i--) {
        window_t* window = wm.windows[i];
        if (!window->visible) {
            continue;
        }

        // Check if mouse is in window bounds
        if (x >= window->x && x < window->x + window->width &&
            y >= window->y && y < window->y + window->height) {
            
            // Title bar interaction
            if (y < window->y + WINDOW_TITLE_HEIGHT) {
                if (buttons & MOUSE_LEFT_BUTTON) {
                    if (!window->dragging) {
                        window_focus(window);
                        window->dragging = true;
                        drag_start_x = x - window->x;
                        drag_start_y = y - window->y;
                    }
                }
            }

            return true;
        }
    }

    // Handle window dragging
    if (wm.focused_window && wm.focused_window->dragging) {
        if (buttons & MOUSE_LEFT_BUTTON) {
            wm.focused_window->x = x - drag_start_x;
            wm.focused_window->y = y - drag_start_y;
        } else {
            wm.focused_window->dragging = false;
        }
        return true;
    }

    return false;
}

void window_focus(window_t* window) {
    if (!wm.initialized || !window) {
        return;
    }

    if (wm.focused_window == window) {
        return;
    }

    // Move window to top
    if (window->id < wm.window_count - 1) {
        for (uint32_t i = window->id; i < wm.window_count - 1; i++) {
            window_t* temp = wm.windows[i];
            wm.windows[i] = wm.windows[i + 1];
            wm.windows[i + 1] = temp;
            wm.windows[i]->id = i;
        }
        window->id = wm.window_count - 1;
    }

    wm.focused_window = window;
}

bool window_is_initialized(void) {
    return wm.initialized;
}

window_t* window_get_focused(void) {
    return wm.focused_window;
}
