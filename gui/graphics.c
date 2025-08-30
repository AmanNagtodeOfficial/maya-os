/**
 * Maya OS Graphics System
 * Updated: 2025-08-29 10:59:28 UTC
 * Author: AmanNagtodeOfficial
 */

#include "gui/graphics.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define VBE_LINEAR_FRAMEBUFFER 0xE0000000
#define DEFAULT_FONT_HEIGHT 16
#define DEFAULT_FONT_WIDTH 8

static struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    bool initialized;
} graphics_state;

// Default 8x16 font data (embedded binary data)
static const uint8_t default_font[256][16] = {
    // Font data would go here
    // Each character is 16 bytes representing 8x16 pixels
};

bool graphics_init(void) {
    if (graphics_state.initialized) {
        return true;
    }

    // Initialize VBE mode
    // This would typically be done by the bootloader
    graphics_state.framebuffer = (uint32_t*)VBE_LINEAR_FRAMEBUFFER;
    graphics_state.width = SCREEN_WIDTH;
    graphics_state.height = SCREEN_HEIGHT;
    graphics_state.pitch = SCREEN_WIDTH * 4;
    graphics_state.bpp = 32;

    // Validate framebuffer
    if (!graphics_state.framebuffer) {
        return false;
    }

    // Clear screen
    graphics_clear(0x000000);

    graphics_state.initialized = true;
    return true;
}

void graphics_clear(uint32_t color) {
    if (!graphics_state.initialized) {
        return;
    }

    for (uint32_t i = 0; i < graphics_state.height; i++) {
        for (uint32_t j = 0; j < graphics_state.width; j++) {
            graphics_state.framebuffer[i * graphics_state.width + j] = color;
        }
    }
}

void graphics_put_pixel(int x, int y, uint32_t color) {
    if (!graphics_state.initialized ||
        x < 0 || x >= (int)graphics_state.width ||
        y < 0 || y >= (int)graphics_state.height) {
        return;
    }

    graphics_state.framebuffer[y * graphics_state.width + x] = color;
}

void graphics_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    if (!graphics_state.initialized) {
        return;
    }

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (true) {
        graphics_put_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}

void graphics_draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (!graphics_state.initialized) {
        return;
    }

    graphics_draw_line(x, y, x + width - 1, y, color);
    graphics_draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);
    graphics_draw_line(x, y, x, y + height - 1, color);
    graphics_draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void graphics_fill_rect(int x, int y, int width, int height, uint32_t color) {
    if (!graphics_state.initialized) {
        return;
    }

    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            graphics_put_pixel(j, i, color);
        }
    }
}

void graphics_draw_char(char c, int x, int y, uint32_t color) {
    if (!graphics_state.initialized) {
        return;
    }

    const uint8_t* char_data = default_font[(unsigned char)c];

    for (int i = 0; i < DEFAULT_FONT_HEIGHT; i++) {
        uint8_t row = char_data[i];
        for (int j = 0; j < DEFAULT_FONT_WIDTH; j++) {
            if (row & (1 << (7 - j))) {
                graphics_put_pixel(x + j, y + i, color);
            }
        }
    }
}

void graphics_draw_text(const char* text, int x, int y, uint32_t color) {
    if (!graphics_state.initialized || !text) {
        return;
    }

    int cursor_x = x;
    int cursor_y = y;

    while (*text) {
        if (*text == '\n') {
            cursor_x = x;
            cursor_y += DEFAULT_FONT_HEIGHT;
        } else if (*text == '\r') {
            cursor_x = x;
        } else {
            graphics_draw_char(*text, cursor_x, cursor_y, color);
            cursor_x += DEFAULT_FONT_WIDTH;
        }
        text++;
    }
}

bool graphics_is_initialized(void) {
    return graphics_state.initialized;
}

uint32_t graphics_get_width(void) {
    return graphics_state.width;
}

uint32_t graphics_get_height(void) {
    return graphics_state.height;
}
