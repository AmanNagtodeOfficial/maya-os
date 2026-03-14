#include "gui/window.h"
#include "gui/graphics.h"
#include "libc/string.h"
#include "libc/stdio.h"

static window_t* hw_win = NULL;

void hardware_manager_draw(window_t* win) {
    if (!win) return;

    // Background
    graphics_fill_rect(win->x, win->y + WINDOW_TITLEBAR_HEIGHT, 
                       win->width, win->height - WINDOW_TITLEBAR_HEIGHT, 
                       MAYA_DESKTOP_BG);

    int text_x = win->x + 20;
    int text_y = win->y + WINDOW_TITLEBAR_HEIGHT + 20;
    int spacing = 25;

    graphics_draw_text("Maya OS Hardware & Device Manager", text_x, text_y, MAYA_TEXT_COLOR);
    graphics_draw_line(text_x, text_y + 15, win->x + win->width - 20, text_y + 15, MAYA_MENU_BORDER);
    
    // List Scaffolds
    text_y += spacing + 10;
    graphics_draw_text("Network Adapters:", text_x, text_y, WIN_BLUE_ACCENT);
    graphics_draw_text("- 802.11 WiFi Interface (Stub)", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    graphics_draw_text("- RTL8139 Fast Ethernet", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    graphics_draw_text("- Bluetooth HCI (Stub)", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    
    text_y += spacing + 10;
    graphics_draw_text("Display Adapters:", text_x, text_y, WIN_BLUE_ACCENT);
    graphics_draw_text("- Standard VGA Graphics Adapter", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    graphics_draw_text("- High-Definition Multimedia Interface (HDMI)", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    graphics_draw_text("- Advanced 3D GPU (Stubbed via Software)", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);

    text_y += spacing + 10;
    graphics_draw_text("Storage Controllers:", text_x, text_y, WIN_BLUE_ACCENT);
    graphics_draw_text("- AHCI SATA Controller", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
    graphics_draw_text("- ATA/ATAPI IDE Controller", text_x + 10, text_y += spacing, MAYA_TEXT_COLOR);
}

void hardware_manager_init(void) {
    if (hw_win != NULL) {
        window_focus(hw_win);
        return;
    }

    hw_win = window_create("Hardware Manager", 150, 100, 450, 400);
    if (hw_win) {
        window_set_draw_callback(hw_win, hardware_manager_draw);
        window_show(hw_win);
    }
}
