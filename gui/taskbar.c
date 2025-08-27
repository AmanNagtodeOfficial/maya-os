#include "gui/taskbar.h"
#include "gui/graphics.h"
#include "kernel/timer.h"
#include "libc/stdio.h"
#include "libc/string.h"

static taskbar_t taskbar;

void taskbar_init(void) {
    taskbar.x = 0;
        taskbar.y = 0;
            taskbar.width = SCREEN_WIDTH;
                taskbar.height = TASKBAR_HEIGHT;
                    taskbar.visible = 1;

                        // Initialize system status
                            taskbar.status.battery_level = 85;
                                taskbar.status.wifi_strength = 3;
                                    taskbar.status.volume_level = 75;

                                        taskbar_update_time();
                                        }

                                        void taskbar_render(void) {
                                            if (!taskbar.visible) return;

                                                // Draw taskbar background (Ubuntu dark theme)
                                                    graphics_fill_rect(taskbar.x, taskbar.y, taskbar.width, taskbar.height,
                                                                       UBUNTU_DARK_GREY);

                                                        // Draw top border
                                                            graphics_draw_line(0, 0, SCREEN_WIDTH, 0, UBUNTU_BORDER_COLOR);

                                                                // Draw Ubuntu logo/activities button
                                                                    graphics_fill_circle(12, 14, 8, UBUNTU_ORANGE);
                                                                        graphics_draw_string(30, 8, "Activities", WHITE);

                                                                            // Draw system tray (right side)
                                                                                int tray_x = SCREEN_WIDTH - SYSTEM_TRAY_WIDTH;

                                                                                    // Draw system indicators
                                                                                        taskbar_draw_indicators(tray_x);

                                                                                            // Draw time and date
                                                                                                taskbar_draw_time(tray_x + 150);
                                                                                                }

                                                                                                static void taskbar_draw_indicators(int x) {
                                                                                                    // WiFi indicator
                                                                                                        for (int i = 0; i < taskbar.status.wifi_strength; i++) {
                                                                                                                graphics_fill_rect(x + i * 3, 20 - i * 2, 2, i * 2 + 4, WHITE);
                                                                                                                    }
                                                                                                                        x += 15;

                                                                                                                            // Battery indicator
                                                                                                                                graphics_draw_rect(x, 8, 20, 12, WHITE);
                                                                                                                                    graphics_fill_rect(x + 20, 11, 2, 6, WHITE);

                                                                                                                                        // Battery fill based on level
                                                                                                                                            int fill_width = (taskbar.status.battery_level * 18) / 100;
                                                                                                                                                graphics_fill_rect(x + 1, 9, fill_width, 10,
                                                                                                                                                                   taskbar.status.battery_level > 20 ? GREEN : RED);
                                                                                                                                                                       x += 30;

                                                                                                                                                                           // Volume indicator
                                                                                                                                                                               graphics_draw_string(x, 8, "â™ª", WHITE);
                                                                                                                                                                                   x += 15;

                                                                                                                                                                                       // User menu indicator
                                                                                                                                                                                           graphics_draw_string(x, 8, "â–¼", WHITE);
                                                                                                                                                                                           }

                                                                                                                                                                                           static void taskbar_draw_time(int x) {
                                                                                                                                                                                               graphics_draw_string(x, 8, taskbar.status.time_str, WHITE);
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
                                                                                                                                                                                                                                    if (y > TASKBAR_HEIGHT) return;

                                                                                                                                                                                                                                        // Activities button
                                                                                                                                                                                                                                            if (x >= 0 && x <= 80) {
                                                                                                                                                                                                                                                    dash_show(); // Show application launcher
                                                                                                                                                                                                                                                            return;
                                                                                                                                                                                                                                                                }

                                                                                                                                                                                                                                                                    // System tray area
                                                                                                                                                                                                                                                                        int tray_x = SCREEN_WIDTH - SYSTEM_TRAY_WIDTH;
                                                                                                                                                                                                                                                                            if (x >= tray_x) {
                                                                                                                                                                                                                                                                                    // Handle system menu clicks
                                                                                                                                                                                                                                                                                        taskbar_show_system_menu();
                                                                                                                                                                                                                                                                                        }
                                                                                                                                                                                                                                                                                        }