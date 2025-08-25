
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

// Color definitions for VGA palette
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GREY 7
#define COLOR_DARK_GREY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15

// Graphics initialization and basic drawing
void graphics_init(void);
void graphics_set_mode(uint8_t mode);
void graphics_clear_screen(uint8_t color);

// Pixel operations
void graphics_set_pixel(int x, int y, uint8_t color);
uint8_t graphics_get_pixel(int x, int y);

// Drawing primitives
void graphics_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint8_t color);
void graphics_fill_rect(int x, int y, int width, int height, uint8_t color);
void graphics_draw_circle(int cx, int cy, int radius, uint8_t color);
void graphics_fill_circle(int cx, int cy, int radius, uint8_t color);

// Text rendering
void graphics_draw_char(int x, int y, char c, uint8_t fg_color, uint8_t bg_color);
void graphics_draw_string(int x, int y, const char *str, uint8_t color);
void graphics_draw_string_bg(int x, int y, const char *str, uint8_t fg_color, uint8_t bg_color);

// Font management
void graphics_set_font(uint8_t *font_data);
void graphics_get_text_size(const char *text, int *width, int *height);

// Double buffering
void graphics_enable_double_buffer(void);
void graphics_disable_double_buffer(void);
void graphics_swap_buffers(void);
void graphics_copy_buffer(void);

// Clipping and viewport
void graphics_set_clip_rect(int x, int y, int width, int height);
void graphics_clear_clip_rect(void);
void graphics_set_viewport(int x, int y, int width, int height);

// Palette operations
void graphics_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void graphics_get_palette_color(uint8_t index, uint8_t *r, uint8_t *g, uint8_t *b);
void graphics_set_palette(uint8_t *palette);

// Screen capture and bitmap operations
void graphics_capture_screen(uint8_t *buffer);
void graphics_draw_bitmap(int x, int y, int width, int height, uint8_t *bitmap);
void graphics_draw_sprite(int x, int y, int width, int height, uint8_t *sprite, uint8_t transparent_color);

// Mouse cursor
void graphics_show_cursor(void);
void graphics_hide_cursor(void);
void graphics_set_cursor_pos(int x, int y);
void graphics_set_cursor_shape(uint8_t *cursor_data);

#endif
