#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/desktop.h"
#include "kernel/memory.h"

static window_t* cp_window = NULL;
static int selected_tab = 0; // 0: Personalization, 1: System

void control_panel_draw_content(void) {
    if (!cp_window || !cp_window->visible) return;

    // Clear background
    window_draw_rect(cp_window, 0, 0, cp_window->width - 4, cp_window->height - WINDOW_TITLE_HEIGHT - 4, 0xF5F5F5);

    // Sidebar
    window_draw_rect(cp_window, 0, 0, 150, cp_window->height - WINDOW_TITLE_HEIGHT - 4, 0xEBEBEB);
    graphics_draw_line(cp_window->x + 150, cp_window->y + WINDOW_TITLE_HEIGHT + 2, cp_window->x + 150, cp_window->y + cp_window->height - 2, 0xCCCCCC);

    // Sidebar Items
    graphics_fill_rect(cp_window->x + 5, cp_window->y + WINDOW_TITLE_HEIGHT + 10 + (selected_tab * 30), 140, 25, MAYA_SELECTED_BG);
    graphics_draw_text("Personalization", cp_window->x + 10, cp_window->y + WINDOW_TITLE_HEIGHT + 15, selected_tab == 0 ? 0xFFFFFF : 0x333333);
    graphics_draw_text("System Info", cp_window->x + 10, cp_window->y + WINDOW_TITLE_HEIGHT + 45, selected_tab == 1 ? 0xFFFFFF : 0x333333);

    // Content Area
    int content_x = cp_window->x + 160;
    int content_y = cp_window->y + WINDOW_TITLE_HEIGHT + 20;

    if (selected_tab == 0) {
        graphics_draw_text("Personalize your OS", content_x, content_y, 0x000000);
        graphics_draw_line(content_x, content_y + 15, cp_window->x + cp_window->width - 20, content_y + 15, 0xCCCCCC);
        graphics_draw_text("1. Windows Dark Theme", content_x, content_y + 30, 0x333333);
        graphics_draw_text("2. Windows Light Theme", content_x, content_y + 60, 0x333333);
        graphics_draw_text("Press '1' or '2' to preview logic", content_x, content_y + 100, 0x888888);
    } else if (selected_tab == 1) {
        graphics_draw_text("System Information", content_x, content_y, 0x000000);
        graphics_draw_line(content_x, content_y + 15, cp_window->x + cp_window->width - 20, content_y + 15, 0xCCCCCC);
        graphics_draw_text("OS: Maya OS v1.0", content_x, content_y + 30, 0x333333);
        graphics_draw_text("UI: Fluent Design Update", content_x, content_y + 50, 0x333333);
        graphics_draw_text("Processor: Generic x86", content_x, content_y + 70, 0x333333);
    }

    window_refresh(cp_window);
}

void control_panel_handle_input(uint32_t key) {
    if (!cp_window || !cp_window->visible) return;

    if (key == 'p' || key == 'P') {
        selected_tab = 0;
    } else if (key == 's' || key == 'S') {
        selected_tab = 1;
    }

    control_panel_draw_content();
}

void control_panel_init(void) {
    cp_window = window_create("Settings - Control Panel", 200, 150, 600, 400);
    if (!cp_window) return;
    
    selected_tab = 0;
    control_panel_draw_content();
}

void control_panel_draw(void) {
    if (cp_window && cp_window->visible) {
        window_render(cp_window);
    }
}
