#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/widgets.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"

static window_t* pkg_window = NULL;

typedef struct {
    char* name;
    char* version;
    char* description;
    bool installed;
} package_t;

static package_t packages[] = {
    {"Terminal", "1.1.0", "System command line interface", true},
    {"Notepad", "1.0.5", "Simple text editor", true},
    {"File Manager", "1.0.2", "VFS browser", true},
    {"Web Browser", "0.9.0", "Basic HTML viewer", false},
    {"Calculator", "1.0.0", "Scientific calculator", false},
    {NULL, NULL, NULL, false}
};

void package_manager_init(void) {
    pkg_window = window_create("Package Manager", 200, 100, 600, 400);
    if (!pkg_window) return;

    window_clear(pkg_window, 0xFFFFFF);

    // List packages
    int y = 20;
    for (int i = 0; packages[i].name != NULL; i++) {
        char buf[256];
        sprintf(buf, "%s v%s - %s [%s]", 
                packages[i].name, 
                packages[i].version, 
                packages[i].description,
                packages[i].installed ? "Installed" : "Click to Install");
        
        label_t* lbl = label_create(10, y, buf);
        window_add_widget(pkg_window, (widget_t*)lbl);
        
        if (!packages[i].installed) {
            button_t* btn = button_create(450, y - 5, 80, 25, "Install");
            window_add_widget(pkg_window, (widget_t*)btn);
        }
        
        y += 40;
    }

    window_refresh(pkg_window);
}

void package_manager_draw(void) {
    if (pkg_window && pkg_window->visible) {
        window_render(pkg_window);
    }
}
