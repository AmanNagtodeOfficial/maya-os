#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/widgets.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"

static window_t* fm_window = NULL;
static char current_path[256] = "/";

// Mock file list for now (until VFS is integrated)
static const char* mock_files[] = {
    "bin", "dev", "etc", "home", "lib", "tmp", "var", "kernel.bin", "readme.txt"
};

void file_manager_init(void) {
    fm_window = window_create("File Manager", 150, 150, 500, 350);
    if (!fm_window) return;

    window_clear(fm_window, 0xF0F0F0); // Light grey background

    // Create a "Back" button
    button_t* back_btn = button_create(10, 10, 60, 25, "Back");
    window_add_widget(fm_window, (widget_t*)back_btn);

    // Current path label
    label_t* path_lbl = label_create(80, 15, current_path);
    window_add_widget(fm_window, (widget_t*)path_lbl);

    // List files
    int y = 50;
    for (int i = 0; i < 9; i++) {
        label_t* file_lbl = label_create(20, y, mock_files[i]);
        window_add_widget(fm_window, (widget_t*)file_lbl);
        y += 20;
    }

    window_refresh(fm_window);
}

void file_manager_draw(void) {
    if (fm_window && fm_window->visible) {
        window_render(fm_window);
        
        // Draw widgets attached to this window
        // In a full implementation, window_t would have a child widget list
        // and we'd call widget_draw on it.
        // For now, we'll manually draw them or assume they are drawn by the window.
    }
}
