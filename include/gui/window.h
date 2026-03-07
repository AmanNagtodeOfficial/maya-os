#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define WINDOW_TITLE_MAX 64

typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_DRAGGING,
    WINDOW_STATE_RESIZING
} window_state_t;

typedef struct {
    int x, y;
    int width, height;
    char title[WINDOW_TITLE_MAX];
    uint32_t *buffer;
    uint8_t visible;
    uint8_t dragging;
    uint8_t resizing;
    window_state_t state;
    uint32_t id;
    
    // Widgets list
    struct widget *widgets;
    
    // Title bar button regions
    int close_x, close_y, close_w, close_h;
    int min_x, min_y, min_w, min_h;
    int max_x, max_y, max_w, max_h;
} window_t;

void window_add_widget(window_t *window, struct widget *widget);
void window_draw_widgets(window_t *window);

window_t *window_create(const char *title, int x, int y, int width, int height);
void window_destroy(window_t *window);
void window_show(window_t *window);
void window_hide(window_t *window);
void window_move(window_t *window, int x, int y);
void window_resize(window_t *window, int width, int height);
void window_set_title(window_t *window, const char *title);
void window_draw_pixel(window_t *window, int x, int y, uint8_t color);
void window_draw_rect(window_t *window, int x, int y, int width, int height, uint8_t color);
void window_clear(window_t *window, uint8_t color);
void window_refresh(window_t *window);

#endif

