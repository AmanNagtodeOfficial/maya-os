/**
 * Maya OS GUI Launcher
 * Updated: 2025-08-29 10:52:34 UTC
 * Author: AmanNagtodeOfficial
 */

#include "gui/launcher.h"
#include "gui/graphics.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define MAX_LAUNCHER_ITEMS 32
#define MIN_ITEM_HEIGHT 32
#define MAX_ITEM_HEIGHT 64

static launcher_t launcher;

bool launcher_init(void) {
    // Clear launcher structure
    memset(&launcher, 0, sizeof(launcher_t));
    
    // Validate screen dimensions
    if (SCREEN_WIDTH < LAUNCHER_WIDTH || SCREEN_HEIGHT < TASKBAR_HEIGHT) {
        return false;
    }
    
    // Initialize launcher properties
    launcher.visible = true;
    launcher.auto_hide = false;
    launcher.item_count = 0;
    launcher.selected_item = -1;
    
    // Add default applications
    if (!launcher_add_default_items()) {
        return false;
    }
    
    return true;
}

bool launcher_add_item(const char *name, const char *icon_path, void (*callback)(void)) {
    if (launcher.item_count >= MAX_LAUNCHER_ITEMS) {
        return false;
    }
    
    launcher_item_t *item = &launcher.items[launcher.item_count];
    
    // Validate parameters
    if (!name || strlen(name) >= MAX_ITEM_NAME_LENGTH) {
        return false;
    }
    
    // Initialize item
    strncpy(item->name, name, MAX_ITEM_NAME_LENGTH - 1);
    item->name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    if (icon_path) {
        strncpy(item->icon_path, icon_path, MAX_PATH_LENGTH - 1);
        item->icon_path[MAX_PATH_LENGTH - 1] = '\0';
    } else {
        item->icon_path[0] = '\0';
    }
    
    item->callback = callback;
    item->enabled = true;
    
    launcher.item_count++;
    return true;
}

void launcher_render(void) {
    if (!launcher.visible) return;
    
    // Draw launcher background
    graphics_fill_rect(0, TASKBAR_HEIGHT, 
                      LAUNCHER_WIDTH,
                      SCREEN_HEIGHT - TASKBAR_HEIGHT,
                      MAYA_LAUNCHER_BG);
    
    // Draw border
    graphics_draw_line(LAUNCHER_WIDTH - 1, TASKBAR_HEIGHT,
                      LAUNCHER_WIDTH - 1, SCREEN_HEIGHT,
                      MAYA_BORDER_COLOR);
    
    // Calculate item dimensions
    int available_height = SCREEN_HEIGHT - TASKBAR_HEIGHT - (2 * LAUNCHER_PADDING);
    int item_height = available_height / launcher.item_count;
    
    // Clamp item height
    if (item_height < MIN_ITEM_HEIGHT) item_height = MIN_ITEM_HEIGHT;
    if (item_height > MAX_ITEM_HEIGHT) item_height = MAX_ITEM_HEIGHT;
    
    // Draw items
    int y = TASKBAR_HEIGHT + LAUNCHER_PADDING;
    
    for (int i = 0; i < launcher.item_count; i++) {
        launcher_item_t *item = &launcher.items[i];
        
        if (!item->enabled) continue;
        
        // Draw item background
        uint32_t bg_color = (i == launcher.selected_item) ? 
                           MAYA_SELECTED_BG : MAYA_LAUNCHER_BG;
        
        graphics_fill_rect(LAUNCHER_PADDING, y,
                          LAUNCHER_WIDTH - (2 * LAUNCHER_PADDING),
                          item_height, bg_color);
        
        // Draw icon if available
        if (item->icon_path[0] != '\0') {
            // TODO: Implement icon rendering
        }
        
        // Draw item name
        graphics_draw_text(item->name,
                          LAUNCHER_PADDING + 40, // Leave space for icon
                          y + (item_height / 2) - 8,
                          MAYA_TEXT_COLOR);
        
        y += item_height;
    }
}

bool launcher_handle_click(int x, int y) {
    if (!launcher.visible) return false;
    
    // Check if click is within launcher bounds
    if (x < 0 || x >= LAUNCHER_WIDTH ||
        y < TASKBAR_HEIGHT || y >= SCREEN_HEIGHT) {
        return false;
    }
    
    // Calculate item height
    int available_height = SCREEN_HEIGHT - TASKBAR_HEIGHT - (2 * LAUNCHER_PADDING);
    int item_height = available_height / launcher.item_count;
    
    // Clamp item height
    if (item_height < MIN_ITEM_HEIGHT) item_height = MIN_ITEM_HEIGHT;
    if (item_height > MAX_ITEM_HEIGHT) item_height = MAX_ITEM_HEIGHT;
    
    // Calculate clicked item
    int item_y = TASKBAR_HEIGHT + LAUNCHER_PADDING;
    
    for (int i = 0; i < launcher.item_count; i++) {
        if (y >= item_y && y < item_y + item_height) {
            if (launcher.items[i].enabled && launcher.items[i].callback) {
                launcher.items[i].callback();
                return true;
            }
            break;
        }
        item_y += item_height;
    }
    
    return false;
}
