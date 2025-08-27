#include "gui/launcher.h"
#include "gui/graphics.h"
#include "kernel/memory.h"
#include "libc/string.h"

static launcher_t launcher;

void launcher_init(void) {
    memset(&launcher, 0, sizeof(launcher_t));
    launcher.visible = 1;
    launcher.auto_hide = 0;

    // Add default applications
    launcher_add_default_items();
}

void launcher_render(void) {
    if (!launcher.visible) return;

    // Draw launcher background (semi-transparent dark)
    graphics_fill_rect(0, TASKBAR_HEIGHT, LAUNCHER_WIDTH,
                           SCREEN_HEIGHT - TASKBAR_HEIGHT,
                           UBUNTU_LAUNCHER_BG);

    // Draw right border
    graphics_draw_line(LAUNCHER_WIDTH - 1, TASKBAR_HEIGHT,
                       LAUNCHER_WIDTH - 1, SCREEN_HEIGHT,
                       UBUNTU_BORDER_COLOR);

    // Draw launcher items
    int y_offset = TASKBAR_HEIGHT + LAUNCHER_PADDING;

    for (int i = 0; i < launcher.item_count; i++) {
        launcher_item_t *item = &launcher.items[i];

        // Calculate item position
        int item_y = y_offset + i * (LAUNCHER_ITEM_SIZE + LAUNCHER_PADDING);
        int item_x = (LAUNCHER_WIDTH - LAUNCHER_ITEM_SIZE) / 2;

        // Skip items that are scrolled out of view
        if (item_y + LAUNCHER_ITEM_SIZE > SCREEN_HEIGHT) break;

        // Draw item background if highlighted or running
        if (item->highlighted || item->running) {
            uint8_t bg_color = item->highlighted ? UBUNTU_HIGHLIGHT : UBUNTU_ACTIVE;
            graphics_fill_rect(item_x - 2, item_y - 2,
                               LAUNCHER_ITEM_SIZE + 4, LAUNCHER_ITEM_SIZE + 4,
                               bg_color);
        }

        // Draw item icon
        if (item->icon_data) {
            graphics_draw_icon(item_x, item_y, item->icon_data);
        } else {
            // Draw default application icon
            graphics_fill_rect(item_x, item_y, LAUNCHER_ITEM_SIZE, LAUNCHER_ITEM_SIZE,
                               UBUNTU_DEFAULT_ICON);
            graphics_draw_string(item_x + 8, item_y + 20, "App", WHITE);
        }

        // Draw running indicator
        if (item->running) {
            graphics_fill_rect(0, item_y + 20, 3, 8, UBUNTU_ORANGE);
        }
    }
}

void launcher_add_item(const char *name, const char *command, uint8_t *icon) {
    if (launcher.item_count >= MAX_LAUNCHER_ITEMS) return;

    launcher_item_t *item = &launcher.items[launcher.item_count];

    item->name = kmalloc(strlen(name) + 1);
    strcpy(item->name, name);

    item->command = kmalloc(strlen(command) + 1);
    strcpy(item->command, command);

    item->icon_data = icon;
    item->pinned = 1;
    item->running = 0;
    item->highlighted = 0;

    launcher.item_count++;
}

static void launcher_add_default_items(void) {
    launcher_add_item("Files", "file_manager", NULL);
    launcher_add_item("Terminal", "terminal", NULL);
    launcher_add_item("Text Editor", "text_editor", NULL);
    launcher_add_item("Calculator", "calculator", NULL);
    launcher_add_item("System Settings", "settings", NULL);
}

void launcher_handle_click(int x, int y) {
    if (x >= LAUNCHER_WIDTH) return;

    // Calculate which item was clicked
    int item_index = (y - TASKBAR_HEIGHT - LAUNCHER_PADDING) /
                     (LAUNCHER_ITEM_SIZE + LAUNCHER_PADDING);

    if (item_index >= 0 && item_index < launcher.item_count) {
        launcher_item_t *item = &launcher.items[item_index];

        // Execute application
        launcher_execute_item(item);
    }
}

static void launcher_execute_item(launcher_item_t *item) {
    // Mark as running
    item->running = 1;

    // Execute the command (simplified)
    if (strcmp(item->command, "file_manager") == 0) {
        file_manager_t *fm = file_manager_create();
        file_manager_open_directory(fm, "/home");
    } else if (strcmp(item->command, "terminal") == 0) {
        // Launch terminal
        terminal_create();
    }
    // Add more application launches as needed
}