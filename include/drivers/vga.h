#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// VGA colors
typedef enum {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} vga_color_t;

#define VGA_WIDTH 80 
#define VGA_HEIGHT 25 

                                                                void vga_init(void);
                                                                void vga_clear(void);
                                                                void vga_putchar(char c);
                                                                void vga_write(const char *data, size_t size);
                                                                void vga_writestring(const char *data);
                                                                void vga_set_color(uint8_t color);
                                                                void vga_set_cursor(size_t x, size_t y);

                                                                // Graphics mode functions
                                                                void vga_set_mode_13h(void);
                                                                void vga_put_pixel(int x, int y, uint8_t color);
                                                                void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
                                                                void vga_draw_rect(int x, int y, int width, int height, uint8_t color);

                                                                #endif
                                                                