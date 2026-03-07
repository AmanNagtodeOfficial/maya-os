#include "gui/desktop.h"
#include "gui/graphics.h"
#include "gui/input.h"
#include "gui/apps.h"
#include "gui/window.h"
#include "gui/taskview.h"
#include "gui/settings_app.h"
#include "kernel/memory.h"
#include "kernel/timer.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "gui/notification.h"

static desktop_t desktop;

void desktop_init(void) {
    printf("Initializing Maya Desktop Environment\n");

    memset(&desktop, 0, sizeof(desktop_t));

    // Set initial UI state
    desktop.show_launcher = 0; // Removing Ubuntu Dock
    desktop.show_taskbar = 1;
    desktop.show_dash = 0;
    desktop.show_start_menu = 0; // New Windows Start Menu
    desktop.show_system_menu = 0;
    desktop.show_network_flyout = 0;
    desktop.show_volume_flyout = 0;
    desktop.show_battery_flyout = 0;

    // Initialize system status
    desktop.battery_level = 85;
    desktop.wifi_strength = 3;
    desktop.volume_level = 75;

    // Add built-in applications
    desktop_add_application("Terminal",         "terminal",       "Accessories");
    desktop_add_application("Notepad",           "notepad",        "Accessories");
    desktop_add_application("Virtual Keyboard",  "virtual_keyboard","Accessories");
    desktop_add_application("File Manager",      "files",          "System");
    desktop_add_application("Control Panel",     "control_panel",  "System");
    desktop_add_application("Date & Time",       "time_applet",    "System");
    desktop_add_application("Settings",          "settings",       "System");

    // Initialize the windows 11 settings panel
    settings_init();

    // Add default desktop icons
    desktop_add_icon("Home",     "/home",   80,  80,  0); // Folder
    desktop_add_icon("Computer", "/",        80,  160, 0); // Folder
    desktop_add_icon("Trash",    "/trash",  80,  240, 0); // Folder
    desktop_add_icon("Notepad",  "",        80,  320, 1); // App shortcut
    desktop_add_icon("Settings", "",        80,  400, 1); // App shortcut
    desktop_add_icon("Terminal", "",        80,  480, 1); // App shortcut

    printf("Maya Desktop initialized with %d icons and %d apps\n",
           desktop.icon_count, desktop.app_count);
}

void desktop_draw_wallpaper(void) {
    // Windows Bloom gradient (Deep Blue to Cyan)
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        uint32_t color = graphics_interpolate_color(DESKTOP_BG_TOP, DESKTOP_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
        graphics_fill_rect(0, y, SCREEN_WIDTH, 1, color);
    }
}

void desktop_draw_taskbar(void) {
    if (!desktop.show_taskbar) return;

    // GNOME Top Bar (Dark, completely opaque for VGA)
    graphics_fill_rect(0, 0, SCREEN_WIDTH, TASKBAR_HEIGHT, UBUNTU_DARK_GREY);
    graphics_draw_line(0, TASKBAR_HEIGHT - 1, SCREEN_WIDTH, TASKBAR_HEIGHT - 1, UBUNTU_BORDER_COLOR);

    // Left side: Activities/Menu
    graphics_draw_text("Activities", 10, 8, MAYA_TEXT_COLOR);

    // Center: Clock
    char clock_text[32];
    sprintf(clock_text, "12:34 PM"); // Placeholder
    graphics_draw_text(clock_text, (SCREEN_WIDTH / 2) - 30, 8, MAYA_TEXT_COLOR);

    // Right side: System status
    graphics_draw_text("85%", SCREEN_WIDTH - 100, 8, MAYA_TEXT_COLOR); // Battery
    graphics_draw_text("Wi-Fi", SCREEN_WIDTH - 60, 8, MAYA_TEXT_COLOR);
}

void desktop_toggle_start_menu(void) {
    desktop.show_start_menu = !desktop.show_start_menu;
    
    // Clear search on open/close
    memset(desktop.start_menu_search, 0, sizeof(desktop.start_menu_search));
    desktop.start_menu_search_len = 0;
    
    if (desktop.show_start_menu) {
        desktop.show_network_flyout = 0;
        desktop.show_volume_flyout = 0;
        desktop.show_battery_flyout = 0;
    }
}

void desktop_toggle_network_flyout(void) {
    desktop.show_network_flyout = !desktop.show_network_flyout;
    if (desktop.show_network_flyout) {
        desktop.show_start_menu = 0;
        desktop.show_volume_flyout = 0;
        desktop.show_battery_flyout = 0;
    }
}

void desktop_toggle_volume_flyout(void) {
    desktop.show_volume_flyout = !desktop.show_volume_flyout;
    if (desktop.show_volume_flyout) {
        desktop.show_start_menu = 0;
        desktop.show_network_flyout = 0;
        desktop.show_battery_flyout = 0;
    }
}

void desktop_toggle_battery_flyout(void) {
    desktop.show_battery_flyout = !desktop.show_battery_flyout;
    if (desktop.show_battery_flyout) {
        desktop.show_start_menu = 0;
        desktop.show_network_flyout = 0;
        desktop.show_volume_flyout = 0;
    }
}

void desktop_draw_start_menu(void) {
    if (!desktop.show_start_menu) return;

    // Draw Start Menu (Centered above taskbar)
    int menu_width = 300;
    int menu_height = 400;
    int menu_x = (SCREEN_WIDTH / 2) - (menu_width / 2);
    int menu_y = SCREEN_HEIGHT - TASKBAR_HEIGHT - menu_height - 10;

    // Background panel with border
    graphics_fill_rect(menu_x, menu_y, menu_width, menu_height, WIN_START_BTN_BG);
    graphics_draw_rect(menu_x, menu_y, menu_width, menu_height, WIN_BORDER_COLOR);

    // Title / Search bar placeholder
    if (desktop.start_menu_search_len > 0) {
        graphics_draw_text(desktop.start_menu_search, menu_x + 15, menu_y + 15, 0xFFFFFF);
    } else {
        graphics_draw_text("Type here to search...", menu_x + 15, menu_y + 15, 0x888888);
    }
    
    graphics_draw_line(menu_x + 10, menu_y + 35, menu_x + menu_width - 10, menu_y + 35, WIN_BORDER_COLOR);

    // Drawing App Grid (Filtered)
    int grid_x = menu_x + 20;
    int grid_y = menu_y + 50;
    for (int i = 0; i < desktop.app_count; i++) {
        // Simple case-insensitive substring search (mocked as simple start match for now)
        if (desktop.start_menu_search_len > 0) {
            int match = 1;
            for (int c = 0; c < desktop.start_menu_search_len; c++) {
                char s1 = desktop.start_menu_search[c];
                char s2 = desktop.apps[i].name[c];
                if (s1 >= 'A' && s1 <= 'Z') s1 += 32;
                if (s2 >= 'A' && s2 <= 'Z') s2 += 32;
                if (s1 != s2) { match = 0; break; }
            }
            if (!match) continue; // Skip rendering if didn't match
        }

        // App icon background
        graphics_fill_rect(grid_x, grid_y, 48, 48, MAYA_SELECTED_BG);
        // First initial as placeholder
        char label[2] = {desktop.apps[i].name[0], '\0'};
        graphics_draw_text(label, grid_x + 20, grid_y + 20, MAYA_TEXT_COLOR);
        // App Name below icon
        graphics_draw_text(desktop.apps[i].name, grid_x, grid_y + 55, MAYA_TEXT_COLOR);

        grid_x += 64; // move right
        if (grid_x > menu_x + menu_width - 64) {
            grid_x = menu_x + 20; // new row
            grid_y += 80;
        }
    }
}

void desktop_handle_start_menu_input(char ascii) {
    if (!desktop.show_start_menu) return;
    
    if (ascii == '\b' && desktop.start_menu_search_len > 0) {
        desktop.start_menu_search[--desktop.start_menu_search_len] = '\0';
    } else if (ascii >= ' ' && ascii <= '~' && desktop.start_menu_search_len < 63) {
        desktop.start_menu_search[desktop.start_menu_search_len++] = ascii;
        desktop.start_menu_search[desktop.start_menu_search_len] = '\0';
    }
}

void desktop_draw_network_flyout(void) {
    if (!desktop.show_network_flyout) return;
    int flyout_w = 250, flyout_h = 200;
    int flyout_x = SCREEN_WIDTH - flyout_w - 10;
    int flyout_y = SCREEN_HEIGHT - TASKBAR_HEIGHT - flyout_h - 10;

    graphics_fill_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_START_BTN_BG);
    graphics_draw_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_BORDER_COLOR);
    graphics_draw_text("Network & Internet", flyout_x + 10, flyout_y + 10, MAYA_TEXT_COLOR);
    graphics_draw_line(flyout_x + 10, flyout_y + 25, flyout_x + flyout_w - 10, flyout_y + 25, WIN_BORDER_COLOR);
    
    graphics_draw_text(desktop.wifi_strength > 0 ? "Wi-Fi: Connected (Maya_Net)" : "Wi-Fi: Disconnected", flyout_x + 10, flyout_y + 40, MAYA_TEXT_COLOR);
    graphics_fill_rect(flyout_x + 10, flyout_y + 70, 80, 40, WIN_BLUE_ACCENT); // Wi-Fi Toggle
    graphics_draw_text("WLAN", flyout_x + 30, flyout_y + 85, 0xFFFFFF);
    
    graphics_fill_rect(flyout_x + 100, flyout_y + 70, 80, 40, 0x444444); // BT Toggle
    graphics_draw_text("Bluetooth", flyout_x + 115, flyout_y + 85, 0xFFFFFF);
    
    graphics_fill_rect(flyout_x + 10, flyout_y + 120, 80, 40, 0x444444); // Hotspot
    graphics_draw_text("Hotspot", flyout_x + 25, flyout_y + 135, 0xFFFFFF);
}

void desktop_draw_volume_flyout(void) {
    if (!desktop.show_volume_flyout) return;
    int flyout_w = 250, flyout_h = 100;
    int flyout_x = SCREEN_WIDTH - flyout_w - 70; // Positioned over Volume
    int flyout_y = SCREEN_HEIGHT - TASKBAR_HEIGHT - flyout_h - 10;

    graphics_fill_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_START_BTN_BG);
    graphics_draw_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_BORDER_COLOR);

    graphics_draw_text("Volume", flyout_x + 10, flyout_y + 10, MAYA_TEXT_COLOR);
    
    // Slider Bar
    graphics_fill_rect(flyout_x + 10, flyout_y + 40, 230, 10, 0x444444);
    int vol_width = (desktop.volume_level * 230) / 100;
    graphics_fill_rect(flyout_x + 10, flyout_y + 40, vol_width, 10, WIN_BLUE_ACCENT);
    
    // EQ Placeholder
    graphics_draw_text("EQ: Flat", flyout_x + 10, flyout_y + 70, 0x888888);
}

void desktop_draw_battery_flyout(void) {
    if (!desktop.show_battery_flyout) return;
    int flyout_w = 200, flyout_h = 130;
    int flyout_x = SCREEN_WIDTH - flyout_w - 40; // Positioned over Battery
    int flyout_y = SCREEN_HEIGHT - TASKBAR_HEIGHT - flyout_h - 10;

    graphics_fill_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_START_BTN_BG);
    graphics_draw_rect(flyout_x, flyout_y, flyout_w, flyout_h, WIN_BORDER_COLOR);
    
    char bat_txt[32];
    sprintf(bat_txt, "Battery: %d%%", desktop.battery_level);
    graphics_draw_text(bat_txt, flyout_x + 10, flyout_y + 15, MAYA_TEXT_COLOR);
    
    graphics_draw_text("Power Mode:", flyout_x + 10, flyout_y + 50, MAYA_TEXT_COLOR);
    // Visual Slider Representation
    graphics_fill_rect(flyout_x + 10, flyout_y + 70, 180, 5, 0x444444);
    graphics_fill_rect(flyout_x + 90, flyout_y + 65, 10, 15, WIN_BLUE_ACCENT); // Thumb center (Balanced)
    
    graphics_draw_text("Best Performance", flyout_x + 50, flyout_y + 95, WIN_BLUE_ACCENT);
}

void desktop_render(void) {
    desktop_draw_wallpaper();
    
    // Draw all windows BEFORE taskbar/start menu so they appear on top
    window_render_all();
    
    desktop_draw_taskbar();
    desktop_draw_start_menu();
    desktop_draw_network_flyout();
    desktop_draw_volume_flyout();
    desktop_draw_battery_flyout();
    
    // Task View overlay (drawn on top of everything)
    taskview_draw(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Notifications
    notification_update();
    notification_render();
    
    // Update screen
    graphics_update();
}


void desktop_add_application(const char *name, const char *command, const char *category) {
    if (desktop.app_count >= MAX_APPLICATIONS) return;
    
    application_t *app = &desktop.apps[desktop.app_count++];
    strncpy(app->name, name, 63);
    strncpy(app->command, command, 127);
    strncpy(app->category, category, 31);
    app->running = 0;
}

void desktop_launch_application(int app_index) {
    if (app_index < 0 || app_index >= desktop.app_count) return;
    application_t *app = &desktop.apps[app_index];
    app->running = 1;
    printf("Launching: %s\n", app->name);

    // Dispatch to known app handlers
    if (strcmp(app->command, "notepad") == 0) {
        extern void notepad_init(void);
        notepad_init();
    } else if (strcmp(app->command, "terminal") == 0) {
        extern void terminal_init(void);
        terminal_init();
    } else if (strcmp(app->command, "virtual_keyboard") == 0) {
        extern void virtual_keyboard_init(void);
        virtual_keyboard_init();
    } else if (strcmp(app->command, "control_panel") == 0) {
        extern void control_panel_init(void);
        control_panel_init();
    } else if (strcmp(app->command, "time_applet") == 0) {
        extern void time_applet_init(void);
        time_applet_init();
    } else if (strcmp(app->command, "settings") == 0) {
        // Settings already initialised at boot; just make its window visible
        extern void settings_draw(void);
        settings_draw();
    } else {
        printf("Unknown app: %s\n", app->command);
    }
}

void desktop_add_icon(const char *name, const char *path, int x, int y, uint8_t type) {
    if (desktop.icon_count >= MAX_DESKTOP_ICONS) return;
    
    desktop_icon_t *icon = &desktop.icons[desktop.icon_count++];
    strncpy(icon->name, name, 63);
    strncpy(icon->path, path, 255);
    icon->x = x;
    icon->y = y;
    icon->width = 48;
    icon->height = 48;
    icon->icon_type = type;
}

void desktop_handle_icon_click(int x, int y) {
    for (int i = 0; i < desktop.icon_count; i++) {
        desktop_icon_t *icon = &desktop.icons[i];
        if (x >= icon->x && x <= icon->x + icon->width &&
            y >= icon->y && y <= icon->y + icon->height) {
            
            // Mark as selected (single click)
            for (int j = 0; j < desktop.icon_count; j++) {
                desktop.icons[j].selected = (i == j);
            }

            // Launch on double click
            extern uint8_t maya_input_is_double_click(void);
            extern void maya_input_clear_double_click(void);
            if (maya_input_is_double_click()) {
                maya_input_clear_double_click();
                printf("Launching from shortcut: %s\n", icon->name);
                // In a real OS, look up the association or App by name/path
                // For now, if there's an app named identically, launch it
                uint8_t launched = 0;
                for (int a = 0; a < desktop.app_count; a++) {
                    if (strcmp(desktop.apps[a].name, icon->name) == 0) {
                        desktop_launch_application(a);
                        launched = 1;
                        break;
                    }
                }
                
                if (!launched) {
                    // Fallback to launching standard file manager
                    for (int a = 0; a < desktop.app_count; a++) {
                        if (strcmp(desktop.apps[a].name, "File Manager") == 0) {
                            desktop_launch_application(a);
                            break;
                        }
                    }
                }
            }
            return;
        }
    }
    
    // Clicked empty space
    for (int j = 0; j < desktop.icon_count; j++) {
        desktop.icons[j].selected = 0;
    }
}