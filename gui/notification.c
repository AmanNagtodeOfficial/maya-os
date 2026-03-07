/**
 * Maya OS Notification Center
 * Updated: 2026-03-07 22:15:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "gui/notification.h"
#include "gui/graphics.h"
#include "kernel/timer.h"
#include "libc/string.h"

static struct {
    notification_t queue[MAX_NOTIFICATIONS];
    uint32_t count;
    bool initialized;
} notification_state;

void notification_init(void) {
    memset(&notification_state, 0, sizeof(notification_state));
    notification_state.initialized = true;
}

void notification_push(const char* title, const char* message) {
    if (!notification_state.initialized) return;

    // Find free slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (!notification_state.queue[i].active) {
            strncpy(notification_state.queue[i].title, title, 63);
            strncpy(notification_state.queue[i].message, message, 127);
            notification_state.queue[i].start_time = timer_get_ticks();
            notification_state.queue[i].active = true;
            notification_state.count++;
            return;
        }
    }
}

void notification_update(void) {
    uint32_t current_time = timer_get_ticks();
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (notification_state.queue[i].active) {
            if (current_time - notification_state.queue[i].start_time > NOTIFICATION_DURATION) {
                notification_state.queue[i].active = false;
                notification_state.count--;
            }
        }
    }
}

void notification_render(void) {
    int y = 20; // Top right margin
    int x = SCREEN_WIDTH - 220;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (notification_state.queue[i].active) {
            // Draw glassmorphism-style toast
            graphics_fill_rect_alpha(x, y, 200, 60, 0x202020, 180);
            graphics_draw_rounded_rect(x, y, 200, 60, 8, 0x444444);
            
            graphics_draw_text(notification_state.queue[i].title, x + 10, y + 10, 0xFFFFFF);
            graphics_draw_text(notification_state.queue[i].message, x + 10, y + 30, 0xCCCCCC);
            
            y += 70; // Spacing between toasts
        }
    }
}
