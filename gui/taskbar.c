#include "gui/taskbar.h"
#include "gui/graphics.h"
#include "kernel/timer.h"
#include "libc/stdio.h"
#include "libc/string.h"

static taskbar_t taskbar;

void taskbar_init(void) {
    taskbar.x = 0;
    taskbar.height = TASKBAR_HEIGHT;
    taskbar.y = SCREEN_HEIGHT - taskbar.height;
    taskbar.width = SCREEN_WIDTH;
    taskbar.visible = 1;

                        // Initialize system status
                            taskbar.status.battery_level = 85;
                                taskbar.status.wifi_strength = 3;
                                    taskbar.status.volume_level = 75;

                                        taskbar_update_time();
                                        }

void taskbar_render(void) {
    if (!taskbar.visible) return;

    extern uint8_t maya_input_is_tablet_mode(void);
    uint8_t is_touch = maya_input_is_tablet_mode();
    int render_height = is_touch ? (taskbar.height + 12) : taskbar.height; // Larger size for fingers
    int render_y = SCREEN_HEIGHT - render_height;

    // Draw taskbar background (Windows dark theme)
    graphics_fill_rect(taskbar.x, render_y, taskbar.width, render_height, WIN_TASKBAR_BG);

    // Draw top border
    graphics_draw_line(0, render_y, SCREEN_WIDTH, render_y, WIN_BORDER_COLOR);

    // Centered App/Start Area
    int start_x = (SCREEN_WIDTH / 2) - 16;
    
    // Draw Windows Start button
    graphics_fill_rect(start_x, taskbar.y + 6, 16, 16, WIN_BLUE_ACCENT);

    // Draw open window apps to the right of the start button
    extern window_t* window_get_by_index(int index);
    extern uint32_t window_get_count(void);
    extern window_t* window_get_focused(void);
    
    int app_x = start_x + 32;
    uint32_t w_count = window_get_count();
    
    for (uint32_t i = 0; i < w_count; i++) {
        window_t* w = window_get_by_index(i);
        if (w) {
            uint32_t bg_color = (w == window_get_focused()) ? 0x444444 : 0x222222;
            graphics_fill_rect(app_x, taskbar.y + 4, 30, 20, bg_color);
            // First letter of window as icon
            char label[2] = {w->title[0], '\0'};
            graphics_draw_text(label, app_x + 10, taskbar.y + 8, 0xFFFFFF);
            
            // Active window underline indicator
            if (w == window_get_focused()) {
                graphics_fill_rect(app_x + 5, taskbar.y + 24, 20, 2, WIN_BLUE_ACCENT);
            }
            
            app_x += 35;
        }
    }

    // Draw system tray (right side)
    int tray_x = SCREEN_WIDTH - SYSTEM_TRAY_WIDTH + 40;
    taskbar_draw_indicators(tray_x);
    taskbar_draw_time(tray_x + 150);
}

static void taskbar_draw_indicators(int x) {
    int y = taskbar.y + 8;
    // WiFi indicator (curved bars)
    for (int i = 0; i < 4; i++) {
        uint32_t color = (i < taskbar.status.wifi_strength) ? 0xFFFFFF : 0x555555;
        graphics_draw_line(x - i*2, y + 10 - i*3, x + i*2, y + 10 - i*3, color);
    }
    x += 20;

    // Battery indicator (modern outline)
    graphics_draw_rect(x, y + 2, 22, 10, 0xFFFFFF);
    graphics_fill_rect(x + 22, y + 5, 2, 4, 0xFFFFFF);
    int fill = (taskbar.status.battery_level * 20) / 100;
    graphics_fill_rect(x + 1, y + 3, fill, 8, (taskbar.status.battery_level > 20) ? 0x00FF00 : 0xFF0000);
    x += 35;

    // Clock handled separately in taskbar_draw_time
}

static void taskbar_draw_time(int x) {
    graphics_draw_text(taskbar.status.time_str, x, taskbar.y + 8, 0xFFFFFF);
}


                                                                                                                                                                                               void taskbar_update_time(void) {
                                                                                                                                                                                                   uint32_t ticks = timer_get_tick();
                                                                                                                                                                                                       uint32_t seconds = ticks / 100; // Assuming 100 Hz timer
                                                                                                                                                                                                           uint32_t minutes = seconds / 60;
                                                                                                                                                                                                               uint32_t hours = (minutes / 60) % 24;
                                                                                                                                                                                                                   minutes = minutes % 60;

                                                                                                                                                                                                                       snprintf(taskbar.status.time_str, sizeof(taskbar.status.time_str),
                                                                                                                                                                                                                                "%02d:%02d", hours, minutes);
                                                                                                                                                                                                                                }

void taskbar_handle_click(int x, int y) {
    if (!taskbar.visible || y < taskbar.y) return;

    // Check Start Button Click (Centered)
    int start_x = (SCREEN_WIDTH / 2) - 16;
    if (x >= start_x && x <= start_x + 32) {
        extern void desktop_toggle_start_menu(void);
        desktop_toggle_start_menu();
        return;
    }

    // Check System Tray Clicks
    int tray_x = SCREEN_WIDTH - SYSTEM_TRAY_WIDTH + 40;
    
    // Wi-Fi (approx width 15)
    if (x >= tray_x && x <= tray_x + 20) {
        extern void desktop_toggle_network_flyout(void);
        desktop_toggle_network_flyout();
        return;
    }
    tray_x += 15;

    // Battery (approx width 30)
    if (x >= tray_x && x <= tray_x + 30) {
        extern void desktop_toggle_battery_flyout(void);
        desktop_toggle_battery_flyout();
        return;
    }
    tray_x += 30;

    // Volume (approx width 15)
    if (x >= tray_x && x <= tray_x + 20) {
        extern void desktop_toggle_volume_flyout(void);
        desktop_toggle_volume_flyout();
        return;
    }
}