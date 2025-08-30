/**
 * Maya OS VGA Text Mode Driver
 * Updated: 2025-08-29 11:10:30 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/vga.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_CURSOR_HIGH 0x0E
#define VGA_CURSOR_LOW 0x0F

static struct {
    uint16_t* buffer;
    uint8_t x;
    uint8_t y;
    uint8_t color;
    bool cursor_enabled;
    bool initialized;
} vga_state;

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void vga_update_cursor(void) {
    if (!vga_state.cursor_enabled) {
        return;
    }

    uint16_t pos = vga_state.y * VGA_WIDTH + vga_state.x;

    outb(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb(VGA_DATA_REGISTER, (pos >> 8) & 0xFF);
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb(VGA_DATA_REGISTER, pos & 0xFF);
}

static void vga_scroll(void) {
    // Move everything up one line
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_state.buffer[y * VGA_WIDTH + x] = 
                vga_state.buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Clear bottom line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_state.buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = 
            vga_entry(' ', vga_state.color);
    }
}

bool vga_init(void) {
    if (vga_state.initialized) {
        return true;
    }

    vga_state.buffer = (uint16_t*)VGA_MEMORY;
    vga_state.x = 0;
    vga_state.y = 0;
    vga_state.color = VGA_COLOR(VGA_WHITE, VGA_BLACK);
    vga_state.cursor_enabled = true;

    // Clear screen
    vga_clear();

    vga_state.initialized = true;
    return true;
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_state.buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_state.color);
        }
    }

    vga_state.x = 0;
    vga_state.y = 0;
    vga_update_cursor();
}

void vga_putchar(char c) {
    if (!vga_state.initialized) {
        return;
    }

    switch (c) {
        case '
':
            vga_state.x = 0;
            vga_state.y++;
            break;

        case '':
            vga_state.x = 0;
            break;

        case '	':
            vga_state.x = (vga_state.x + 8) & ~7;
            break;

        case '':
            if (vga_state.x > 0) {
                vga_state.x--;
            } else if (vga_state.y > 0) {
                vga_state.y--;
                vga_state.x = VGA_WIDTH - 1;
            }
            vga_state.buffer[vga_state.y * VGA_WIDTH + vga_state.x] = 
                vga_entry(' ', vga_state.color);
            break;

        default:
            vga_state.buffer[vga_state.y * VGA_WIDTH + vga_state.x] = 
                vga_entry(c, vga_state.color);
            vga_state.x++;
            break;
    }

    if (vga_state.x >= VGA_WIDTH) {
        vga_state.x = 0;
        vga_state.y++;
    }

    if (vga_state.y >= VGA_HEIGHT) {
        vga_scroll();
        vga_state.y = VGA_HEIGHT - 1;
    }

    vga_update_cursor();
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_writestring(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_set_color(uint8_t color) {
    vga_state.color = color;
}

void vga_set_cursor(uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }

    vga_state.x = x;
    vga_state.y = y;
    vga_update_cursor();
}

void vga_enable_cursor(void) {
    vga_state.cursor_enabled = true;
    vga_update_cursor();
}

void vga_disable_cursor(void) {
    vga_state.cursor_enabled = false;
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 0x20);
}

bool vga_is_initialized(void) {
    return vga_state.initialized;
}
