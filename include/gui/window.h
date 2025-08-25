#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

typedef struct {
    int x, y;
    int width, height;
    char *title;
    uint8_t *buffer;
    uint8_t visible;
    uint32_t id;
} window_t;

window_t *window_create(int x, int y, int width, int height, const char *title);
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

