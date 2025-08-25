#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

void graphics_init(void);
void graphics_set_pixel(int x, int y, uint8_t color);
void graphics_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint8_t color);
void graphics_fill_rect(int x, int y, int width, int height, uint8_t color);
void graphics_draw_circle(int cx, int cy, int radius, uint8_t color);
void graphics_draw_string(int x, int y, const char *str, uint8_t color);
void graphics_clear_screen(uint8_t color);

#endif


