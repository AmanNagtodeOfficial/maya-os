
#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "gui/window.h"
#include "gui/graphics.h"

#define MAX_DESKTOP_ICONS 32
#define TASKBAR_HEIGHT 28
#define LAUNCHER_WIDTH 64
#define MAX_APPLICATIONS 32
#define MAX_NOTIFICATIONS 8
#define NOTIFICATION_WIDTH 300
#define NOTIFICATION_HEIGHT 80
#define NOTIFICATION_DURATION 5000

// Color Scheme (using Maya colors as they are more defined)
#define DESKTOP_PURPLE 0x4C1D4F
#define DESKTOP_ORANGE 0xFF8C00
#define DESKTOP_BLUE 0x0084C7
#define DESKTOP_DARK_GREY 0x2C2C2C
#define DESKTOP_LIGHT_GREY 0xF2F1F0
#define DESKTOP_BORDER_COLOR 0x1E1E1E
#define DESKTOP_LAUNCHER_BG 0x332D2D
#define DESKTOP_HIGHLIGHT 0x4A90D9
#define DESKTOP_ACTIVE 0x5E2750
#define DESKTOP_DEFAULT_ICON 0x6A6A6A
#define DESKTOP_FOLDER_COLOR 0xE8A317
#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define GREEN 0x00FF00
#define RED 0xFF0000

typedef struct {
	int x, y;
	int width, height;
	char name[64]; // Increased size to match maya_icon_t
	char path[256]; // Increased size to match maya_icon_t
	uint8_t *icon_data; // Keep the original icon data field
	uint8_t selected;
	uint8_t icon_type; // 0=folder, 1=app, 2=file (from maya_icon_t)
} desktop_icon_t;

// Application Structure (from maya_app_t)
typedef struct {
	char name[64];
	char command[128];
	char category[32];
	uint8_t running;
	uint8_t pinned;
	uint32_t window_id;
} application_t;

// Notification Structure (from maya_notification_t)
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
} notification_t;

// Main Desktop Structure (merged from desktop_t and maya_desktop_t)
typedef struct {
	// Desktop icons
	desktop_icon_t icons[MAX_DESKTOP_ICONS];
	int icon_count;

	// Applications
	application_t apps[MAX_APPLICATIONS];
	int app_count;

	// Notifications
	notification_t notifications[MAX_NOTIFICATIONS];
	int notification_count;

	// UI State
	uint8_t *wallpaper; // Keep the original wallpaper field
	uint8_t show_launcher;
	uint8_t show_taskbar;
	uint8_t show_dash; // From maya_desktop_t
	uint8_t show_system_menu; // From maya_desktop_t

	// Mouse simulation (From maya_desktop_t, consider if needed)
	int mouse_x;
	int mouse_y;
	uint8_t mouse_clicked;

	// Dash state (From maya_desktop_t)
	char search_query[128];
	int selected_app;
	int active_category;

	// System status (From maya_desktop_t)
	uint8_t battery_level;
	uint8_t wifi_strength;
	uint8_t volume_level;
	char time_string[16];
	char date_string[32];
} desktop_t;

// Function Declarations (merged and renamed from maya_desktop.h)
void desktop_init(void);
void desktop_render(void);
void desktop_handle_keyboard(char key);
void desktop_handle_mouse(int x, int y, uint8_t clicked);
void desktop_update(void);

// Component Functions (renamed from maya_*)
void desktop_draw_wallpaper(void);
void desktop_draw_taskbar(void);
void desktop_draw_launcher(void);
void desktop_draw_dash(void);
void desktop_draw_notifications(void);
void desktop_draw_desktop_icons(void);

// Icon Functions (merged and renamed from maya_*)
void desktop_add_icon(const char *name, const char *path, int x, int y, uint8_t type); // Added type
void desktop_handle_icon_click(int x, int y);

// Application Functions (renamed from maya_*)
void desktop_add_application(const char *name, const char *command, const char *category);
void desktop_launch_application(int app_index);
application_t *desktop_find_application(const char *name);

// Notification Functions (renamed from maya_*)
void desktop_show_notification(const char *title, const char *message, notification_type_t type);
void desktop_update_notifications(void);

// System Functions (renamed from maya_*)
void desktop_update_system_status(void);
void desktop_handle_system_menu(void);
void desktop_system_shutdown(void);
void desktop_system_restart(void);

// Drawing Utilities (renamed from maya_*)
void desktop_draw_gradient_rect(int x, int y, int width, int height, uint32_t color1, uint32_t color2);
void desktop_draw_rounded_rect(int x, int y, int width, int height, uint8_t color, int radius);
void desktop_draw_icon(int x, int y, uint8_t icon_type);
uint8_t desktop_color_blend(uint8_t color1, uint8_t color2, float ratio);








#endif
