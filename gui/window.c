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

#define TITLE_BUTTON_SIZE 14
#define TITLE_BUTTON_MARGIN 3

// Modern Color Palette (Windows 11 Fluent)
#define COLOR_TITLE_BG_ACTIVE   0x202020 // Dark grey for focus
#define COLOR_TITLE_BG_INACTIVE 0x2D2D2D
#define COLOR_BORDER_ACTIVE     0x444444
#define COLOR_BORDER_INACTIVE   0x666666
#define COLOR_BUTTON_CLOSE      0xE81123 // Windows Red
#define COLOR_BUTTON_MIN        0x555555 // Dark Grey
#define COLOR_BUTTON_MAX        0x555555 // Dark Grey

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
    window->state = WINDOW_STATE_NORMAL;
    window->id = wm.window_count++;

    // Initialize button regions (relative to window top-right)
    window->close_w = window->close_h = TITLE_BUTTON_SIZE;
    window->close_y = TITLE_BUTTON_MARGIN;
    window->close_x = window->width - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;

    window->max_w = window->max_h = TITLE_BUTTON_SIZE;
    window->max_y = TITLE_BUTTON_MARGIN;
    window->max_x = window->close_x - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;

    window->min_w = window->min_h = TITLE_BUTTON_SIZE;
    window->min_y = TITLE_BUTTON_MARGIN;
    window->min_x = window->max_x - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;

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
                      (window == wm.focused_window) ? COLOR_BORDER_ACTIVE : COLOR_BORDER_INACTIVE);

    // Draw title bar
    uint32_t title_bg = (window == wm.focused_window) ? COLOR_TITLE_BG_ACTIVE : COLOR_TITLE_BG_INACTIVE;
    graphics_fill_rect(window->x + WINDOW_BORDER_WIDTH,
                      window->y + WINDOW_BORDER_WIDTH,
                      window->width - (2 * WINDOW_BORDER_WIDTH),
                      WINDOW_TITLE_HEIGHT,
                      title_bg);

    // Draw title text (Centered in Windows style)
    graphics_draw_text(window->title,
                      window->x + 10,
                      window->y + (WINDOW_TITLE_HEIGHT / 2) - 4,
                      0xFFFFFF);

    // Draw Control Buttons
    // Close (X)
    graphics_fill_rect(window->x + window->close_x, window->y + window->close_y,
                      window->close_w, window->close_h, COLOR_BUTTON_CLOSE);
    graphics_draw_text("X", window->x + window->close_x + 3, window->y + window->close_y + 3, 0xFFFFFF);

    // Maximize ([])
    graphics_fill_rect(window->x + window->max_x, window->y + window->max_y,
                      window->max_w, window->max_h, COLOR_BUTTON_MAX);
    graphics_draw_text("O", window->x + window->max_x + 3, window->y + window->max_y + 3, 0xFFFFFF);

    // Minimize (_)
    graphics_fill_rect(window->x + window->min_x, window->y + window->min_y,
                      window->min_w, window->min_h, COLOR_BUTTON_MIN);
    graphics_draw_text("-", window->x + window->min_x + 3, window->y + window->min_y + 3, 0xFFFFFF);

    // Draw window contents
    graphics_blit(window->buffer,
                  window->x + WINDOW_BORDER_WIDTH,
                  window->y + WINDOW_BORDER_WIDTH + WINDOW_TITLE_HEIGHT,
                  window->width - (2 * WINDOW_BORDER_WIDTH),
                  window->height - (2 * WINDOW_BORDER_WIDTH) - WINDOW_TITLE_HEIGHT);

    // Draw widgets
    window_draw_widgets(window);
}

void window_add_widget(window_t* window, struct widget* widget) {
    if (!window || !widget) return;
    
    if (!window->widgets) {
        window->widgets = widget;
    } else {
        struct widget* last = window->widgets;
        while (last->next) last = last->next;
        last->next = widget;
    }
}

void window_draw_widgets(window_t* window) {
    if (!window) return;
    struct widget* curr = window->widgets;
    while (curr) {
        widget_draw(curr, window);
        curr = curr->next;
    }
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
                int rel_x = x - window->x;
                int rel_y = y - window->y;

                if (buttons & MOUSE_LEFT_BUTTON) {
                    // Check buttons first
                    if (rel_x >= window->close_x && rel_x < window->close_x + window->close_w &&
                        rel_y >= window->close_y && rel_y < window->close_y + window->close_h) {
                        window_destroy(window);
                        return true;
                    }
                    if (rel_x >= window->max_x && rel_x < window->max_x + window->max_w &&
                        rel_y >= window->max_y && rel_y < window->max_y + window->max_h) {
                        // Toggle maximize (placeholder for now)
                        return true;
                    }
                    if (rel_x >= window->min_x && rel_x < window->min_x + window->min_w &&
                        rel_y >= window->min_y && rel_y < window->min_y + window->min_h) {
                        window->visible = false;
                        return true;
                    }

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

void window_show(window_t* window) {
    if (window) window->visible = true;
}

void window_hide(window_t* window) {
    if (window) window->visible = false;
}

void window_move(window_t* window, int x, int y) {
    if (window) {
        window->x = x;
        window->y = y;
    }
}

void window_resize(window_t* window, int width, int height) {
    if (!window || width < MIN_WINDOW_WIDTH || height < MIN_WINDOW_HEIGHT) {
        return;
    }

    uint32_t* new_buffer = kmalloc(width * height * sizeof(uint32_t));
    if (!new_buffer) return;

    memset(new_buffer, 0, width * height * sizeof(uint32_t));
    
    if (window->buffer) {
        // Copy old contents if possible (simplified for now)
        kfree(window->buffer);
    }

    window->buffer = new_buffer;
    window->width = width;
    window->height = height;

    // Reposition buttons
    window->close_x = window->width - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;
    window->max_x = window->close_x - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;
    window->min_x = window->max_x - TITLE_BUTTON_SIZE - TITLE_BUTTON_MARGIN;
}

void window_set_title(window_t* window, const char* title) {
    if (window && title) {
        strncpy(window->title, title, WINDOW_TITLE_MAX - 1);
        window->title[WINDOW_TITLE_MAX - 1] = '\0';
    }
}

void window_draw_rect(window_t* window, int x, int y, int width, int height, uint32_t color) {
    if (!window || !window->buffer) return;
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            window_draw_pixel(window, j, i, color);
        }
    }
}

void window_refresh(window_t* window) {
    window_render(window);
}

void window_focus(window_t* window) {
    if (!wm.initialized || !window) {
        return;
    }

    if (wm.focused_window == window) {
        return;
    }

    // Move window to top
    for (uint32_t i = 0; i < wm.window_count; i++) {
        if (wm.windows[i] == window) {
            for (uint32_t j = i; j < wm.window_count - 1; j++) {
                wm.windows[j] = wm.windows[j + 1];
                wm.windows[j]->id = j;
            }
            wm.windows[wm.window_count - 1] = window;
            window->id = wm.window_count - 1;
            break;
        }
    }

    wm.focused_window = window;
}

bool window_is_initialized(void) {
    return wm.initialized;
}

window_t* window_get_focused(void) {
    return wm.focused_window;
}

uint32_t window_get_count(void) {
    return wm.initialized ? wm.window_count : 0;
}

window_t* window_get_by_index(int index) {
    if (!wm.initialized || index < 0 || index >= wm.window_count) return NULL;
    return wm.windows[index];
}
