#include "/include/gui/maya_desktop.h"
#include "/include/gui/maya_graphics.h"
#include "/include/gui/maya_input.h"
#include "/include/gui/maya_apps.h"
#include "kernel/memory.h"
#include "kernel/timer.h"
#include "libc/string.h"
#include "libc/stdio.h"

// Global desktop instance
static maya_desktop_t desktop;

void maya_desktop_init(void) {
    printf("Initializing Maya Desktop Environment\\n");
    
    // Initialize desktop structure
    memset(&desktop, 0, sizeof(maya_desktop_t));
    
    // Set initial UI state
    desktop.show_launcher = 1;
    desktop.show_taskbar = 1;
    desktop.show_dash = 0;
    desktop.show_system_menu = 0;
    
    // Initialize mouse position
    desktop.mouse_x = SCREEN_WIDTH / 2;
    desktop.mouse_y = SCREEN_HEIGHT / 2;
    
    // Initialize system status
    desktop.battery_level = 85;
    desktop.wifi_strength = 3;
    desktop.volume_level = 75;
    
    // Initialize graphics and input
    maya_graphics_init();
    maya_input_init();
    maya_apps_init();
    
    // Add default desktop icons
    maya_add_desktop_icon("Home", "/home", 80, 80, 0);
    maya_add_desktop_icon("Documents", "/home/documents", 80, 160, 0);
    maya_add_desktop_icon("Pictures", "/home/pictures", 80, 240, 0);
    maya_add_desktop_icon("Music", "/home/music", 180, 80, 0);
    maya_add_desktop_icon("Videos", "/home/videos", 180, 160, 0);
    maya_add_desktop_icon("Computer", "/", 180, 240, 0);
    maya_add_desktop_icon("Trash", "/trash", 280, 80, 0);
    
    // Add default applications
    maya_add_application("Files", "files", CATEGORY_SYSTEM);
    maya_add_application("Terminal", "terminal", CATEGORY_ACCESSORIES);
    maya_add_application("Text Editor", "text-editor", CATEGORY_ACCESSORIES);
    maya_add_application("Calculator", "calculator", CATEGORY_ACCESSORIES);
    maya_add_application("Settings", "settings", CATEGORY_SYSTEM);
    maya_add_application("Web Browser", "web-browser", CATEGORY_INTERNET);
    maya_add_application("Image Viewer", "image-viewer", CATEGORY_GRAPHICS);
    maya_add_application("Music Player", "music-player", CATEGORY_SOUND_VIDEO);
    maya_add_application("Video Player", "video-player", CATEGORY_SOUND_VIDEO);
    maya_add_application("Software Center", "software-center", CATEGORY_SYSTEM);
    
    maya_show_notification("Welcome", "Maya Desktop loaded successfully!", NOTIF_SUCCESS);
    
    printf("Maya Desktop initialized with %d icons and %d apps\\n", 
           desktop.icon_count, desktop.app_count);
}

void ubuntu_desktop_render(void) {
    // Draw wallpaper first
    ubuntu_draw_wallpaper();
    
    // Draw desktop icons
    ubuntu_draw_desktop_icons();
    
    // Draw taskbar (top panel)
    if (desktop.show_taskbar) {
        ubuntu_draw_taskbar();
    }
    
    // Draw launcher (left dock)
    if (desktop.show_launcher) {
        ubuntu_draw_launcher();
    }
    
    // Draw dash overlay if visible
    if (desktop.show_dash) {
        ubuntu_draw_dash();
    }
    
    // Draw notifications
    ubuntu_draw_notifications();
    
    // Draw mouse cursor
    ubuntu_fill_rect(desktop.mouse_x, desktop.mouse_y, 2, 2, WHITE);
    ubuntu_set_pixel(desktop.mouse_x + 1, desktop.mouse_y + 1, BLACK);
}

void ubuntu_draw_wallpaper(void) {
    // Create Ubuntu purple-to-orange gradient wallpaper
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float ratio = (float)y / SCREEN_HEIGHT;
            
            // Ubuntu signature colors: Purple (#4C1D4F) to Orange (#FF8C00)
            uint8_t r = (uint8_t)(0x4C * (1.0f - ratio) + 0xFF * ratio);
            uint8_t g = (uint8_t)(0x1D * (1.0f - ratio) + 0x8C * ratio);
            uint8_t b = (uint8_t)(0x4F * (1.0f - ratio) + 0x00 * ratio);
            
            uint8_t color = ubuntu_rgb_to_color(r, g, b);
            ubuntu_set_pixel(x, y, color);
        }
    }
}

void ubuntu_draw_taskbar(void) {
void maya_draw_taskbar(void) {
    // Draw taskbar background
    ubuntu_fill_rect(0, 0, SCREEN_WIDTH, TASKBAR_HEIGHT, UBUNTU_DARK_GREY);
    ubuntu_draw_line(0, TASKBAR_HEIGHT - 1, SCREEN_WIDTH, TASKBAR_HEIGHT - 1, UBUNTU_BORDER_COLOR);
    
    // Draw Activities button
    ubuntu_fill_rect(8, 4, 70, 20, UBUNTU_DARK_GREY);
    ubuntu_draw_string(12, 8, "Activities", WHITE, UBUNTU_DARK_GREY);
    
    // Draw Maya logo (simplified circle)
    ubuntu_fill_circle(18, 14, 4, UBUNTU_ORANGE);
    
    // Draw window buttons (center area)
    int button_x = 100;
    for (int i = 0; i < desktop.app_count; i++) {
        if (desktop.apps[i].running) {
            uint8_t bg_color = (desktop.apps[i].window_id > 0) ? UBUNTU_HIGHLIGHT : UBUNTU_ACTIVE;
            ubuntu_fill_rect(button_x, 2, 120, 24, bg_color);
            ubuntu_draw_string(button_x + 8, 8, desktop.apps[i].name, WHITE, bg_color);
            button_x += 125;
            
            if (button_x > SCREEN_WIDTH - 200) break;
        }
    }
    
    // Draw system tray (right side)
    int tray_x = SCREEN_WIDTH - 180;
    
    // Network indicator
    for (int i = 0; i < desktop.wifi_strength; i++) {
        ubuntu_fill_rect(tray_x + i * 3, 18 - i * 2, 2, i * 2 + 6, WHITE);
    }
    tray_x += 20;
    
    // Battery indicator
    ubuntu_draw_rect(tray_x, 8, 16, 10, WHITE);
    ubuntu_fill_rect(tray_x + 16, 11, 2, 4, WHITE);
    int battery_fill = (desktop.battery_level * 14) / 100;
    uint8_t battery_color = (desktop.battery_level > 20) ? GREEN : RED;
    ubuntu_fill_rect(tray_x + 1, 9, battery_fill, 8, battery_color);
    tray_x += 25;
    
    // Volume indicator
    ubuntu_draw_string(tray_x, 8, "â™ª", WHITE, UBUNTU_DARK_GREY);
    tray_x += 15;
    
    // Time display
    maya_update_system_status();
    ubuntu_draw_string(tray_x, 6, desktop.time_string, WHITE, UBUNTU_DARK_GREY);
    ubuntu_draw_string(tray_x, 16, desktop.date_string, UBUNTU_LIGHT_GREY, UBUNTU_DARK_GREY);
}

void maya_draw_launcher(void) {
    // Draw launcher background
    maya_fill_rect(0, TASKBAR_HEIGHT, LAUNCHER_WIDTH, SCREEN_HEIGHT - TASKBAR_HEIGHT, MAYA_LAUNCHER_BG);
    ubuntu_draw_line(LAUNCHER_WIDTH - 1, TASKBAR_HEIGHT, LAUNCHER_WIDTH - 1, SCREEN_HEIGHT, UBUNTU_BORDER_COLOR);
    maya_draw_line(LAUNCHER_WIDTH - 1, TASKBAR_HEIGHT, LAUNCHER_WIDTH - 1, SCREEN_HEIGHT, MAYA_BORDER_COLOR);
    // Draw application icons
    int icon_y = TASKBAR_HEIGHT + 8;
    int icon_size = 48;
    int icon_padding = 8;
    
    for (int i = 0; i < desktop.app_count && i < 12; i++) {
        int icon_x = (LAUNCHER_WIDTH - icon_size) / 2;
        
        // Highlight if running
        if (desktop.apps[i].running) {
            uint8_t bg_color = (desktop.apps[i].window_id > 0) ? UBUNTU_HIGHLIGHT : UBUNTU_ACTIVE;
            ubuntu_fill_rect(icon_x - 2, icon_y - 2, icon_size + 4, icon_size + 4, bg_color);
        }
        
        // Draw application icon
        ubuntu_draw_app_icon(icon_x, icon_y, icon_size, desktop.apps[i].name);
        
        // Draw running indicator
        if (desktop.apps[i].running) {
            maya_fill_circle(4, icon_y + icon_size / 2, 3, MAYA_ORANGE);
        }
        
        icon_y += icon_size + icon_padding;
    }
}
void ubuntu_draw_dash(void) {
    // Draw semi-transparent overlay
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            uint8_t bg_color = ubuntu_get_pixel(x, y);
            uint8_t overlay_color = ubuntu_color_blend(bg_color, BLACK, 0.7f);
            ubuntu_set_pixel(x, y, overlay_color);
        }
    }
    
    // Calculate dash window
    int dash_width = SCREEN_WIDTH - 100;
    int dash_height = SCREEN_HEIGHT - 100;
    int dash_x = 50;
    int dash_y = 50;
    
    // Draw dash background
    ubuntu_fill_rect(dash_x, dash_y, dash_width, dash_height, UBUNTU_LIGHT_GREY);
    ubuntu_draw_rect(dash_x, dash_y, dash_width, dash_height, UBUNTU_BORDER_COLOR);
    
    // Draw title
    ubuntu_draw_string(dash_x + 20, dash_y + 20, "Search your computer", BLACK, UBUNTU_LIGHT_GREY);
    
    // Draw search box
    int search_x = dash_x + 20;
    int search_y = dash_y + 50;
    int search_width = dash_width - 40;
    
    ubuntu_fill_rect(search_x, search_y, search_width, 30, WHITE);
    ubuntu_draw_rect(search_x, search_y, search_width, 30, UBUNTU_BORDER_COLOR);
    
    if (strlen(desktop.search_query) > 0) {
        ubuntu_draw_string(search_x + 10, search_y + 8, desktop.search_query, BLACK, WHITE);
    } else {
        ubuntu_draw_string(search_x + 10, search_y + 8, "Type to search...", UBUNTU_DARK_GREY, WHITE);
    }
    
    // Draw application grid
    int grid_x = dash_x + 20;
    int grid_y = dash_y + 100;
    int app_size = 80;
    int apps_per_row = 6;
    
    int app_index = 0;
    for (int row = 0; row < 3 && app_index < desktop.app_count; row++) {
        for (int col = 0; col < apps_per_row && app_index < desktop.app_count; col++) {
            int app_x = grid_x + col * (app_size + 10);
            int app_y = grid_y + row * (app_size + 40);
            
            // Highlight selected app
            if (app_index == desktop.selected_app) {
                ubuntu_fill_rect(app_x - 5, app_y - 5, app_size + 10, app_size + 30, UBUNTU_HIGHLIGHT);
            }
            
            // Draw app icon
            ubuntu_draw_app_icon(app_x, app_y, app_size, desktop.apps[app_index].name);
            
            // Draw app name
            ubuntu_draw_string_centered(app_x, app_y + app_size + 5, app_size, 
                                        desktop.apps[app_index].name, BLACK, UBUNTU_LIGHT_GREY);
            
            app_index++;
        }
    }
}

void maya_draw_desktop_icons(void) {
    for (int i = 0; i < desktop.icon_count; i++) {
        ubuntu_icon_t* icon = &desktop.icons[i];
        
        // Skip icons behind launcher or taskbar
        if ((icon->x < LAUNCHER_WIDTH && desktop.show_launcher) ||
            (icon->y < TASKBAR_HEIGHT && desktop.show_taskbar)) {
            continue;
        }
        
        // Draw selection highlight
        if (icon->selected) {
            ubuntu_fill_rect(icon->x - 5, icon->y - 5, icon->width + 10, icon->height + 25, UBUNTU_HIGHLIGHT);
        }
        
        // Draw icon based on type
        ubuntu_draw_icon(icon->x, icon->y, icon->icon_type);
        
        // Draw icon label with shadow
        ubuntu_draw_string(icon->x + 1, icon->y + icon->height + 6, icon->name, BLACK, 0);
        ubuntu_draw_string(icon->x, icon->y + icon->height + 5, icon->name, WHITE, 0);
    }
}

void maya_draw_notifications(void) {
    ubuntu_update_notifications();
    
    int notif_x = SCREEN_WIDTH - NOTIFICATION_WIDTH - 20;
    int notif_y = TASKBAR_HEIGHT + 20;
    
    for (int i = 0; i < desktop.notification_count; i++) {
        ubuntu_notification_t* notif = &desktop.notifications[i];
        
        if (!notif->visible) continue;
        
        // Choose notification color based on type
        uint8_t bg_color;
        switch (notif->type) {
            case NOTIF_ERROR: bg_color = RED; break;
            case NOTIF_WARNING: bg_color = UBUNTU_ORANGE; break;
            case NOTIF_SUCCESS: bg_color = GREEN; break;
            default: bg_color = UBUNTU_DARK_GREY; break;
        }
        
        // Draw notification background
        ubuntu_fill_rect(notif_x, notif_y, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT, bg_color);
        ubuntu_draw_rect(notif_x, notif_y, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT, UBUNTU_BORDER_COLOR);
        
        // Draw notification text
        ubuntu_draw_string(notif_x + 10, notif_y + 10, notif->title, WHITE, bg_color);
        ubuntu_draw_string(notif_x + 10, notif_y + 25, notif->message, WHITE, bg_color);
        
        // Draw close button
        ubuntu_draw_string(notif_x + NOTIFICATION_WIDTH - 15, notif_y + 5, "Ã—", WHITE, bg_color);
        
        notif_y += NOTIFICATION_HEIGHT + 10;
    }
}

void maya_desktop_handle_keyboard(char key) {
    if (desktop.show_dash) {
        // Handle dash input
        if (key == 27) { // ESC
            desktop.show_dash = 0;
            memset(desktop.search_query, 0, sizeof(desktop.search_query));
        } else if (key == '\n') { // Enter
            if (desktop.selected_app < desktop.app_count) {
                ubuntu_launch_application(desktop.selected_app);
                desktop.show_dash = 0;
            }
        } else if (key >= 32 && key <= 126) {
            // Add character to search
            int len = strlen(desktop.search_query);
            if (len < 127) {
                desktop.search_query[len] = key;
                desktop.search_query[len + 1] = '\0';
            }
        } else if (key == '\b') {
            // Backspace
            int len = strlen(desktop.search_query);
            if (len > 0) {
                desktop.search_query[len - 1] = '\0';
            }
        }
    } else {
        // Handle desktop shortcuts
        switch (key) {
            case ' ': // Space - toggle dash
                desktop.show_dash = !desktop.show_dash;
                desktop.selected_app = 0;
                memset(desktop.search_query, 0, sizeof(desktop.search_query));
                break;
                
            case 'h': // Toggle launcher
                desktop.show_launcher = !desktop.show_launcher;
                break;
                
            case 't': // Toggle taskbar
                desktop.show_taskbar = !desktop.show_taskbar;
                break;
                
            case 27: // ESC - close overlays
                desktop.show_dash = 0;
                desktop.show_system_menu = 0;
                break;
        }
    }
    
    // Handle mouse movement with arrow keys
    switch (key) {
        case 'A': // Up (from arrow key)
            if (desktop.mouse_y > 0) desktop.mouse_y -= 5;
            break;
        case 'B': // Down
            if (desktop.mouse_y < SCREEN_HEIGHT - 1) desktop.mouse_y += 5;
            break;
        case 'C': // Right
            if (desktop.mouse_x < SCREEN_WIDTH - 1) desktop.mouse_x += 5;
            break;
        case 'D': // Left
            if (desktop.mouse_x > 0) desktop.mouse_x -= 5;
            break;
    }
}

void maya_desktop_handle_mouse(int x, int y, uint8_t clicked) {
    desktop.mouse_x = x;
    desktop.mouse_y = y;
    
    if (clicked) {
        if (desktop.show_dash) {
            // Handle dash clicks
            int dash_x = 50;
            int dash_y = 50;
            int dash_width = SCREEN_WIDTH - 100;
            int dash_height = SCREEN_HEIGHT - 100;
            
            if (x < dash_x || x > dash_x + dash_width || y < dash_y || y > dash_y + dash_height) {
                desktop.show_dash = 0; // Click outside closes dash
            } else {
                // Check application grid clicks
                int grid_x = dash_x + 20;
                int grid_y = dash_y + 100;
                int app_size = 80;
                int apps_per_row = 6;
                
                int clicked_app = -1;
                for (int row = 0; row < 3; row++) {
                    for (int col = 0; col < apps_per_row; col++) {
                        int app_x = grid_x + col * (app_size + 10);
                        int app_y = grid_y + row * (app_size + 40);
                        
                        if (x >= app_x && x < app_x + app_size && 
                            y >= app_y && y < app_y + app_size) {
                            clicked_app = row * apps_per_row + col;
                            break;
                        }
                    }
                    if (clicked_app >= 0) break;
                }
                
                if (clicked_app >= 0 && clicked_app < desktop.app_count) {
                    ubuntu_launch_application(clicked_app);
                    desktop.show_dash = 0;
                }
            }
        } else {
            // Handle taskbar clicks
            if (y < TASKBAR_HEIGHT && desktop.show_taskbar) {
                if (x >= 8 && x <= 78) {
                    // Activities button clicked
                    desktop.show_dash = !desktop.show_dash;
                    desktop.selected_app = 0;
                }
            }
            
            // Handle launcher clicks
            if (x < LAUNCHER_WIDTH && desktop.show_launcher && y > TASKBAR_HEIGHT) {
                int icon_y = TASKBAR_HEIGHT + 8;
                int icon_size = 48;
                int icon_padding = 8;
                
                for (int i = 0; i < desktop.app_count && i < 12; i++) {
                    if (y >= icon_y && y < icon_y + icon_size) {
                        ubuntu_launch_application(i);
                        break;
                    }
                    icon_y += icon_size + icon_padding;
                }
            }
            
            // Handle desktop icon clicks
            if (x >= LAUNCHER_WIDTH || !desktop.show_launcher) {
                ubuntu_handle_icon_click(x, y);
            }
        }
    }
}

void maya_desktop_update(void) {
    maya_update_system_status();
    maya_update_notifications();
    maya_input_update();
    
    // Handle input events
    while (maya_input_has_events()) {
        maya_input_event_t event = maya_input_get_event();
        
        switch (event.type) {
            case INPUT_KEY_PRESS:
                ubuntu_desktop_handle_keyboard(event.data.key.ascii);
                break;
                
            case INPUT_MOUSE_CLICK:
                ubuntu_desktop_handle_mouse(event.data.mouse.x, event.data.mouse.y, 1);
                break;
                
            case INPUT_MOUSE_MOVE:
                ubuntu_desktop_handle_mouse(event.data.mouse.x, event.data.mouse.y, 0);
                break;
        }
    }
}

// Icon management functions
void maya_add_desktop_icon(const char* name, const char* path, int x, int y, uint8_t type) {
    if (desktop.icon_count >= MAX_DESKTOP_ICONS) return;
    
    maya_icon_t* icon = &desktop.icons[desktop.icon_count];
    strncpy(icon->name, name, 63);
    icon->name[63] = '\0';
    strncpy(icon->path, path, 255);
    icon->path[255] = '\0';
    
    icon->x = x;
    icon->y = y;
    icon->width = 48;
    icon->height = 48;
    icon->selected = 0;
    icon->icon_type = type;
    
    desktop.icon_count++;
}

void maya_handle_icon_click(int x, int y) {
    // Clear all selections first
    for (int i = 0; i < desktop.icon_count; i++) {
        desktop.icons[i].selected = 0;
    }

    // Check which icon was clicked
    for (int i = 0; i < desktop.icon_count; i++) {
        ubuntu_icon_t* icon = &desktop.icons[i];
        
        if (x >= icon->x && x < icon->x + icon->width &&
            y >= icon->y && y < icon->y + icon->height + 20) {
            icon->selected = 1;
            
            // Show notification
            char msg[128];
            snprintf(msg, sizeof(msg), "Selected %s", icon->name);
            ubuntu_show_notification("Desktop", msg, NOTIF_INFO);
            break;
        }
    }
}

// Application management functions
void maya_add_application(const char* name, const char* command, const char* category) {
    if (desktop.app_count >= MAX_APPLICATIONS) return;
    
    maya_app_t* app = &desktop.apps[desktop.app_count];
    strncpy(app->name, name, 63);
    app->name[63] = '\0';
    strncpy(app->command, command, 127);
    app->command[127] = '\0';
    strncpy(app->category, category, 31);
    app->category[31] = '\0';
    
    app->running = 0;
    app->pinned = 1;
    app->window_id = 0;
    
    desktop.app_count++;
}

void maya_launch_application(int app_index) {
    if (app_index < 0 || app_index >= desktop.app_count) return;
    
    maya_app_t* app = &desktop.apps[app_index];
    
    if (app->running) {
        // Application already running, focus it
        ubuntu_show_notification("Focus", app->name, NOTIF_INFO);
    } else {
        // Launch new application instance
        app->running = 1;
        
        // Create application window based on type (using old function names temporarily)
        ubuntu_app_window_t* app_window = NULL;
        if (strcmp(app->command, "files") == 0) {
            app_window = ubuntu_app_create(APP_FILES, 100, 50, 600, 400);
        } else if (strcmp(app->command, "terminal") == 0) {
            app_window = ubuntu_app_create(APP_TERMINAL, 150, 80, 500, 350);
        } else if (strcmp(app->command, "text-editor") == 0) {
            app_window = ubuntu_app_create(APP_TEXT_EDITOR, 120, 70, 550, 450);
        } else if (strcmp(app->command, "calculator") == 0) {
            app_window = ubuntu_app_create(APP_CALCULATOR, 200, 100, 300, 400);
        } else if (strcmp(app->command, "settings") == 0) {
            app_window = ubuntu_app_create(APP_SETTINGS, 180, 90, 500, 500);
        }
        // Add more application launches as needed
        
        if (app_window) {
            app->window_id = window_get_id(app_window->window);
            ubuntu_show_notification("Launch", app->name, NOTIF_SUCCESS);
        } else {
            app->running = 0; // Mark as not running if window creation failed
            ubuntu_show_notification("Error", "Failed to launch application", NOTIF_ERROR);
        }
    }
}

maya_app_t* maya_find_application(const char* name) {
    for (int i = 0; i < desktop.app_count; i++) {
        if (strcmp(desktop.apps[i].name, name) == 0) {
            return &desktop.apps[i];
        }
    }
    return NULL;
}

// Notification management functions
void maya_show_notification(const char* title, const char* message, notification_type_t type) {
    // Find an available slot
    int index = -1;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (!desktop.notifications[i].visible) {
            index = i;
            break;
        }
    }
    
    // If no slot is available, replace the oldest or do nothing
    if (index == -1) {
        // Simple approach: replace the first one
        index = 0;
    }
    
    maya_notification_t* notif = &desktop.notifications[index];
    strncpy(notif->title, title, 63);
    notif->title[63] = '\0';
    strncpy(notif->message, message, 127);
    notif->message[127] = '\0';
    notif->type = type;
    notif->timestamp = timer_get_tick(); // Get current time
    notif->visible = 1;
    
    if (index >= desktop.notification_count) {
        desktop.notification_count = index + 1;
    }
}

void maya_update_notifications(void) {
    uint32_t current_tick = timer_get_tick();
    for (int i = 0; i < desktop.notification_count; i++) {
        if (desktop.notifications[i].visible) {
            // Check if duration has passed
            if (current_tick - desktop.notifications[i].timestamp > MAYA_NOTIFICATION_DURATION / (1000 / timer_get_frequency())) {
                desktop.notifications[i].visible = 0;
            }
        }
    }
}


// System status functions
void maya_update_system_status(void) {
    uint32_t ticks = timer_get_tick();
    uint32_t seconds = ticks / timer_get_frequency();
    uint32_t minutes = seconds / 60;
    uint32_t hours = (minutes / 60) % 24;
    minutes = minutes % 60;
    
    snprintf(desktop.time_string, sizeof(desktop.time_string), "%02lu:%02lu", hours, minutes);
    
    // Placeholder date (replace with actual RTC if available)
    snprintf(desktop.date_string, sizeof(desktop.date_string), "Jan 01");
}

void maya_handle_system_menu(void) {
    // Placeholder for system menu logic (shutdown, restart, logout)
    desktop.show_system_menu = !desktop.show_system_menu;
    if (desktop.show_system_menu) {
        maya_show_notification("System Menu", "System menu opened", NOTIF_INFO);
    } else {
        maya_show_notification("System Menu", "System menu closed", NOTIF_INFO);
    }
}

void maya_system_shutdown(void) {
    // Placeholder for shutdown logic
    maya_show_notification("System", "Shutting down Maya OS...", NOTIF_INFO);
    // Implement actual shutdown mechanism (e.g., ACPI)
    asm volatile("hlt"); // Halt the CPU as a temporary measure
}

void maya_system_restart(void) {
    // Placeholder for restart logic
    maya_show_notification("System", "Restarting Maya OS...", NOTIF_INFO);
    // Implement actual restart mechanism (e.g., keyboard controller reset)
    asm volatile("int $0x19"); // BIOS restart interrupt (might not work in protected mode)
    asm volatile("hlt"); // Halt if restart fails
}

// Drawing utilities (Simplified, relying on graphics.c)
void maya_draw_gradient_rect(int x, int y, int width, int height, uint32_t color1, uint32_t color2) {
    // Placeholder - relies on the wallpaper gradient logic or similar custom implementation
    // For now, just fill with the first color
    maya_fill_rect(x, y, width, height, color1);
}

void ubuntu_draw_rounded_rect(int x, int y, int width, int height, uint8_t color, int radius) {
    // Placeholder - requires more complex drawing logic
    ubuntu_draw_rect(x, y, width, height, color);
    ubuntu_fill_rect(x, y, width, height, color); // Simple fill for now
}
void ubuntu_draw_icon(int x, int y, uint8_t icon_type) {
    // Use existing graphics functions or define new icon drawing functions
    switch (icon_type) {
        case 0: // Folder icon
            maya_draw_folder_icon(x, y, 48);
            break;
        case 1: // App icon (placeholder)
            ubuntu_fill_rect(x, y, 48, 48, UBUNTU_DEFAULT_ICON);
            ubuntu_draw_string(x + 10, y + 20, "APP", WHITE, UBUNTU_DEFAULT_ICON);
            maya_fill_rect(x, y, 48, 48, MAYA_DEFAULT_ICON);
            maya_draw_string(x + 10, y + 20, "APP", WHITE, MAYA_DEFAULT_ICON);
            break;
        case 2: // File icon (