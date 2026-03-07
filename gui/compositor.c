/**
 * Maya OS Software Compositor
 * Updated: 2026-03-07 21:40:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "gui/compositor.h"
#include "gui/graphics.h"
#include "gui/window.h"
#include "kernel/memory.h"
#include "libc/string.h"

static uint32_t* back_buffer = NULL;
static uint32_t screen_w, screen_h;

void compositor_init(void) {
    screen_w = graphics_get_width();
    screen_h = graphics_get_height();
    back_buffer = kmalloc(screen_w * screen_h * sizeof(uint32_t));
    memset(back_buffer, 0, screen_w * screen_h * sizeof(uint32_t));
}

void compositor_begin_frame(void) {
    // Fill back buffer with desktop background
    // In a real impl, we'd use a desktop wallpaper or color
    memset(back_buffer, 0, screen_w * screen_h * sizeof(uint32_t));
}

void compositor_end_frame(void) {
    // Blit back buffer to the actual framebuffer
    graphics_blit(back_buffer, 0, 0, screen_w, screen_h, screen_w);
}

void compositor_update_region(int x, int y, int w, int h) {
    // Simplified: in a full impl, we'd only redraw windows in this region
    (void)x; (void)y; (void)w; (void)h;
}
