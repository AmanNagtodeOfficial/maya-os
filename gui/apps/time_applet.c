#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "kernel/timer.h" // Assuming this gives time logic
#include "libc/stdio.h"

static window_t* time_window = NULL;
static int adjust_hours = 0;
static int adjust_mins = 0;

void time_applet_draw_content(void) {
    if (!time_window || !time_window->visible) return;

    // Clear background
    window_draw_rect(time_window, 0, 0, time_window->width - 4, time_window->height - WINDOW_TITLE_HEIGHT - 4, 0xFFFFFF);

    // Title 
    int content_x = time_window->x + 20;
    int content_y = time_window->y + WINDOW_TITLE_HEIGHT + 20;

    graphics_draw_text("Date & Time Settings", content_x, content_y, 0x000000);
    graphics_draw_line(content_x, content_y + 15, time_window->x + time_window->width - 20, content_y + 15, 0xCCCCCC);

    // Render Current Time (simulated adjustment)
    char time_str[32];
    sprintf(time_str, "Adjusted Time: %02d:%02d", (12 + adjust_hours) % 24, (34 + adjust_mins) % 60);
    graphics_draw_text(time_str, content_x, content_y + 40, WIN_BLUE_ACCENT);

    // Controls
    graphics_draw_text("[H] +Hour   [h] -Hour", content_x, content_y + 80, 0x333333);
    graphics_draw_text("[M] +Min    [m] -Min", content_x, content_y + 100, 0x333333);

    // Calendar Placeholder
    graphics_draw_rect(content_x + 180, content_y + 30, 150, 120, 0xCCCCCC);
    graphics_draw_text("Calendar View", content_x + 200, content_y + 50, 0x888888);

    window_refresh(time_window);
}

void time_applet_handle_input(uint32_t key) {
    if (!time_window || !time_window->visible) return;

    if (key == 'H') adjust_hours++;
    if (key == 'h') adjust_hours--;
    if (key == 'M') adjust_mins++;
    if (key == 'm') adjust_mins--;

    time_applet_draw_content();
}

void time_applet_init(void) {
    time_window = window_create("Date and Time", 300, 200, 400, 250);
    if (!time_window) return;

    time_applet_draw_content();
}

void time_applet_draw(void) {
    if (time_window && time_window->visible) {
        window_render(time_window);
    }
}
