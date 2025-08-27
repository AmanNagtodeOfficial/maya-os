#ifndef MAYA_DESKTOP_H
#define MAYA_DESKTOP_H

#include <stdint.h>
#include "gui/window.h"
#include "gui/graphics.h"

// Maya Desktop Constants
#define MAX_DESKTOP_ICONS 32
#define TASKBAR_HEIGHT 28
#define LAUNCHER_WIDTH 64
#define MAX_APPLICATIONS 32
#define MAX_NOTIFICATIONS 8
#define NOTIFICATION_WIDTH 300
#define NOTIFICATION_HEIGHT 80#define NOTIFICATION_DURATION 5000

// Maya Color Scheme
#define MAYA_PURPLE 0x4C1D4F
#define MAYA_ORANGE 0xFF8C00
#define MAYA_BLUE 0x0084C7
#define MAYA_DARK_GREY 0x2C2C2C
#define MAYA_LIGHT_GREY 0xF2F1F0
#define MAYA_BORDER_COLOR 0x1E1E1E
#define MAYA_LAUNCHER_BG 0x332D2D
#define MAYA_HIGHLIGHT 0x4A90D9
#define MAYA_ACTIVE 0x5E2750
#define MAYA_DEFAULT_ICON 0x6A6A6A
#define MAYA_FOLDER_COLOR 0xE8A317
#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define GREEN 0x00FF00
#define RED 0xFF0000

// Desktop Icon Structure
typedef struct {
 int x, y;
        int width, height;
            char name[64];
                char path[256];
                    uint8_t selected;
                        uint8_t icon_type; // 0=folder, 1=app, 2=file
 } maya_icon_t;

                        // Application Structure
                        typedef struct {
                            char name[64];
                                char command[128];
                                    char category[32];
                                        uint8_t running;
                                            uint8_t pinned;
                                                uint32_t window_id;
 } maya_app_t;

                                                // Notification Structure
                                                typedef enum {
                                                    NOTIF_INFO,
                                                        NOTIF_WARNING,
                                                            NOTIF_ERROR,
                                                                NOTIF_SUCCESS
                                                                } notification_type_t;

                                                                typedef struct {
                                                                    char title[64];
                                                                        char message[128];
                                                                            notification_type_t type;
                                                                                uint32_t timestamp;
 uint8_t visible;
 } maya_notification_t;

                                                                                    // Main Desktop Structure
                                                                                    typedef struct {
                                                                                        // Desktop icons
 maya_icon_t icons[MAX_DESKTOP_ICONS];
                                                                                                int icon_count;

                                                                                                        // Applications
                                                                                                            ubuntu_app_t apps[MAX_APPLICATIONS];
                                                                                                                int app_count;
                                                                                                                    
                                                                                                                        // Notifications
                                                                                                                            ubuntu_notification_t notifications[MAX_NOTIFICATIONS];
                                                                                                                                int notification_count;
                                                                                                                                    
                                                                                                                                        // UI State
                                                                                                                                            uint8_t show_launcher;
                                                                                                                                                uint8_t show_taskbar;
                                                                                                                                                    uint8_t show_dash;
                                                                                                                                                        uint8_t show_system_menu;
                                                                                                                                                            
                                                                                                                                                                // Mouse simulation
                                                                                                                                                                    int mouse_x;
                                                                                                                                                                        int mouse_y;
                                                                                                                                                                            uint8_t mouse_clicked;
                                                                                                                                                                                
                                                                                                                                                                                    // Dash state
                                                                                                                                                                                        char search_query[128];
                                                                                                                                                                                            int selected_app;
                                                                                                                                                                                                int active_category;
                                                                                                                                                                                                    
                                                                                                                                                                                                        // System status
                                                                                                                                                                                                            uint8_t battery_level;
                                                                                                                                                                                                                uint8_t wifi_strength;
                                                                                                                                                                                                                    uint8_t volume_level;
                                                                                                                                                                                                                        char time_string[16];
                                                                                                                                                                                                                            char date_string[32];
                                                                                                                                                                                                                            } ubuntu_desktop_t;
 } maya_desktop_t;

                                                                                                                                                                                                                            // Function Declarations
                                                                                                                                                                                                                            void maya_desktop_init(void);
                                                                                                                                                                                                                            void maya_desktop_render(void);
                                                                                                                                                                                                                            void maya_desktop_handle_keyboard(char key);
                                                                                                                                                                                                                            void maya_desktop_handle_mouse(int x, int y, uint8_t clicked);
                                                                                                                                                                                                                            void maya_desktop_update(void);

                                                                                                                                                                                                                            // Component Functions
                                                                                                                                                                                                                            void maya_draw_wallpaper(void);
                                                                                                                                                                                                                            void maya_draw_taskbar(void);
                                                                                                                                                                                                                            void maya_draw_launcher(void);
                                                                                                                                                                                                                            void maya_draw_dash(void);
                                                                                                                                                                                                                            void maya_draw_notifications(void);
                                                                                                                                                                                                                            void maya_draw_desktop_icons(void);

                                                                                                                                                                                                                            // Icon Functions
                                                                                                                                                                                                                            void maya_add_desktop_icon(const char* name, const char* path, int x, int y, uint8_t type);
                                                                                                                                                                                                                            void maya_handle_icon_click(int x, int y);

                                                                                                                                                                                                                            // Application Functions
                                                                                                                                                                                                                            void maya_add_application(const char* name, const char* command, const char* category);
                                                                                                                                                                                                                            void maya_launch_application(int app_index);
                                                                                                                                                                                                                            maya_app_t* maya_find_application(const char* name);

                                                                                                                                                                                                                            // Notification Functions
                                                                                                                                                                                                                            void maya_show_notification(const char* title, const char* message, notification_type_t type);
                                                                                                                                                                                                                            void maya_update_notifications(void);

                                                                                                                                                                                                                            // System Functions
                                                                                                                                                                                                                            void maya_update_system_status(void);
                                                                                                                                                                                                                            void maya_handle_system_menu(void);
                                                                                                                                                                                                                            void maya_system_shutdown(void);
                                                                                                                                                                                                                            void maya_system_restart(void);

                                                                                                                                                                                                                            // Drawing Utilities
                                                                                                                                                                                                                            void maya_draw_gradient_rect(int x, int y, int width, int height, uint32_t color1, uint32_t color2);
                                                                                                                                                                                                                            void maya_draw_rounded_rect(int x, int y, int width, int height, uint8_t color, int radius);
                                                                                                                                                                                                                            void maya_draw_icon(int x, int y, uint8_t icon_type);
                                                                                                                                                                                                                            uint8_t maya_color_blend(uint8_t color1, uint8_t color2, float ratio);

                                                                                                                                                                                                                            #endif
