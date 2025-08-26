#ifndef UBUNTU_GRAPHICS_H
#define UBUNTU_GRAPHICS_H

#include <stdint.h>

// Enhanced Graphics Functions for Ubuntu GUI
void ubuntu_graphics_init(void);
void ubuntu_set_pixel(int x, int y, uint8_t color);
uint8_t ubuntu_get_pixel(int x, int y);
void ubuntu_fill_rect(int x, int y, int width, int height, uint8_t color);
void ubuntu_draw_rect(int x, int y, int width, int height, uint8_t color);
void ubuntu_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void ubuntu_fill_circle(int cx, int cy, int radius, uint8_t color);
void ubuntu_draw_circle(int cx, int cy, int radius, uint8_t color);

// Text Rendering
void ubuntu_draw_char(int x, int y, char c, uint8_t color, uint8_t bg_color);
void ubuntu_draw_string(int x, int y, const char* str, uint8_t color, uint8_t bg_color);
void ubuntu_draw_string_centered(int x, int y, int width, const char* str, uint8_t color, uint8_t bg_color);

// Icon Drawing
void ubuntu_draw_folder_icon(int x, int y, int size);
void ubuntu_draw_app_icon(int x, int y, int size, const char* app_name);
void ubuntu_draw_file_icon(int x, int y, int size);
void ubuntu_draw_system_icon(int x, int y, const char* icon_name, uint8_t color);

// Effects
void ubuntu_draw_shadow(int x, int y, int width, int height);
void ubuntu_draw_glow(int x, int y, int width, int height, uint8_t color);
void ubuntu_apply_transparency(int x, int y, int width, int height, float alpha);

// Color Utilities
uint8_t ubuntu_rgb_to_color(uint8_t r, uint8_t g, uint8_t b);
void ubuntu_color_to_rgb(uint8_t color, uint8_t* r, uint8_t* g, uint8_t* b);
uint8_t ubuntu_darken_color(uint8_t color, float factor);
uint8_t ubuntu_lighten_color(uint8_t color, float factor);

// Animation Support
typedef struct {
    int start_value;
        int end_value;
            int current_value;
                uint32_t start_time;
                    uint32_t duration;
                        uint8_t active;
                        } ubuntu_animation_t;

                        void ubuntu_animation_start(ubuntu_animation_t* anim, int start, int end, uint32_t duration);
                        void ubuntu_animation_update(ubuntu_animation_t* anim);
                        uint8_t ubuntu_animation_finished(ubuntu_animation_t* anim);

                        #endif
