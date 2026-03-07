/**
 * Maya OS - Settings Panel
 * Windows 11-style Settings with all major sections
 * Author: AmanNagtodeOfficial
 */

#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/input.h"
#include "gui/apps.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "kernel/memory.h"

// ── Colors ──────────────────────────────────────────────────────────────────
#define SETTINGS_BG         0x1C1C1C
#define SETTINGS_SIDEBAR_BG 0x141414
#define SETTINGS_PANEL_BG   0x202020
#define SETTINGS_HEADER_BG  0x252525
#define SETTINGS_HOVER_BG   0x333333
#define SETTINGS_ACTIVE_BG  0x003D7A
#define SETTINGS_TEXT_PRI   0xFFFFFF
#define SETTINGS_TEXT_SEC   0xAAAAAA
#define SETTINGS_ACCENT     0x0078D4
#define SETTINGS_TOGGLE_ON  0x0078D4
#define SETTINGS_TOGGLE_OFF 0x555555
#define SETTINGS_DIVIDER    0x333333
#define SETTINGS_CARD_BG    0x2A2A2A

// ── Section identifiers ──────────────────────────────────────────────────────
#define SEC_HOME          0
#define SEC_SYSTEM        1
#define SEC_BLUETOOTH     2
#define SEC_NETWORK       3
#define SEC_PERSONALIZE   4
#define SEC_APPS          5
#define SEC_ACCOUNTS      6
#define SEC_TIME_LANGUAGE 7
#define SEC_ACCESSIBILITY 8
#define SEC_PRIVACY       9
#define SEC_UPDATE        10
#define SEC_COUNT         11

// ── Layout ─────────────────────────────────────────────────────────────────
#define SIDEBAR_W  220
#define HEADER_H   50
#define ITEM_H     40
#define PANEL_PAD  18

static const char *SECTION_LABELS[SEC_COUNT] = {
    "Home",
    "System",
    "Bluetooth & devices",
    "Network & internet",
    "Personalization",
    "Apps",
    "Accounts",
    "Time & language",
    "Accessibility",
    "Privacy & security",
    "Windows Update"
};

// Icons (first-letter) – in real HW renderer these would be glyphs
static const char SECTION_ICONS[SEC_COUNT] = {
    'H', 'S', 'B', 'N', 'P', 'A', 'U', 'T', 'W', '#', 'U'
};

typedef struct {
    window_t *win;
    int       active_section;
    int       scroll_y;
    // Personalization toggles
    uint8_t   dark_mode;
    uint8_t   transparency;
    uint8_t   animations;
    int       accent_color_index;
    // System
    uint8_t   night_light;
    uint8_t   focus_assist;
    int       display_scale;   // 100, 125, 150
    int       volume;          // 0-100
    int       brightness;      // 0-100
    // Network
    uint8_t   wifi_enabled;
    uint8_t   bluetooth_enabled;
    uint8_t   airplane_mode;
    uint8_t   hotspot_enabled;
    // Privacy
    uint8_t   location;
    uint8_t   camera;
    uint8_t   microphone;
    uint8_t   diagnostic;
    // Update
    uint8_t   auto_update;
    uint8_t   update_available;
    char      last_checked[32];
    // Accounts
    char      user_name[32];
    char      user_email[64];
} settings_state_t;

static settings_state_t g_settings;

// ── Helpers ─────────────────────────────────────────────────────────────────

static void draw_toggle(int x, int y, uint8_t on) {
    uint32_t track_col = on ? SETTINGS_TOGGLE_ON : SETTINGS_TOGGLE_OFF;
    graphics_fill_rect(x, y + 5, 40, 14, track_col);
    // Thumb
    int thumb_x = on ? x + 24 : x + 2;
    graphics_fill_rect(thumb_x, y + 3, 18, 18, SETTINGS_TEXT_PRI);
}

static void draw_slider(int x, int y, int val, int max_val) {
    int track_w = 200;
    int fill_w  = (val * track_w) / (max_val ? max_val : 1);
    graphics_fill_rect(x, y + 7, track_w, 6, SETTINGS_HOVER_BG);
    graphics_fill_rect(x, y + 7, fill_w, 6, SETTINGS_ACCENT);
    // Thumb
    graphics_fill_rect(x + fill_w - 6, y + 1, 12, 18, SETTINGS_TEXT_PRI);
}

static void draw_card_row(int x, int y, int w, const char *label,
                          const char *desc, uint8_t toggle, uint8_t *state_ptr) {
    graphics_fill_rect(x, y, w, 54, SETTINGS_CARD_BG);
    graphics_draw_text(label, x + 15, y + 10, SETTINGS_TEXT_PRI);
    if (desc && desc[0]) {
        graphics_draw_text(desc, x + 15, y + 30, SETTINGS_TEXT_SEC);
    }
    if (toggle) {
        draw_toggle(x + w - 60, y + 16, *state_ptr);
    }
    // Bottom divider
    graphics_draw_line(x, y + 54, x + w, y + 54, SETTINGS_DIVIDER);
}

static void draw_section_header(int x, int y, int w, const char *title) {
    graphics_fill_rect(x, y, w, 32, SETTINGS_HEADER_BG);
    graphics_draw_text(title, x + 12, y + 9, SETTINGS_ACCENT);
    graphics_draw_line(x, y + 32, x + w, y + 32, SETTINGS_DIVIDER);
}

// ── Section renderers ────────────────────────────────────────────────────────

static void render_home(int px, int py, int pw) {
    // User card at top
    graphics_fill_rect(px, py, pw, 80, SETTINGS_CARD_BG);
    graphics_fill_rect(px + 14, py + 14, 52, 52, SETTINGS_ACCENT);  // Avatar circle
    graphics_draw_text(g_settings.user_name[0] ? g_settings.user_name : "User",
                       px + 25, py + 32, SETTINGS_TEXT_PRI);
    graphics_draw_text(g_settings.user_name,  px + 80, py + 20, SETTINGS_TEXT_PRI);
    graphics_draw_text(g_settings.user_email, px + 80, py + 40, SETTINGS_TEXT_SEC);

    int cy = py + 100;
    draw_section_header(px, cy, pw, "Quick settings"); cy += 40;

    draw_card_row(px, cy, pw, "Night Light",     "Reduces blue light at night", 1, &g_settings.night_light);  cy += 60;
    draw_card_row(px, cy, pw, "Focus Assist",    "Suppress notifications",       1, &g_settings.focus_assist); cy += 60;
    draw_card_row(px, cy, pw, "Transparency",    "Acrylic / mica effects",       1, &g_settings.transparency); cy += 60;
    draw_card_row(px, cy, pw, "Dark Mode",       "Use dark theme throughout",    1, &g_settings.dark_mode);    cy += 60;
    draw_card_row(px, cy, pw, "Animations",      "Enable window animations",     1, &g_settings.animations);   cy += 60;
}

static void render_system(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Display"); cy += 40;

    // Brightness
    graphics_fill_rect(px, cy, pw, 64, SETTINGS_CARD_BG);
    graphics_draw_text("Brightness", px + 15, cy + 10, SETTINGS_TEXT_PRI);
    char bval[8]; sprintf(bval, "%d%%", g_settings.brightness);
    graphics_draw_text(bval, px + pw - 50, cy + 10, SETTINGS_TEXT_SEC);
    draw_slider(px + 15, cy + 32, g_settings.brightness, 100); cy += 70;

    // Volume
    graphics_fill_rect(px, cy, pw, 64, SETTINGS_CARD_BG);
    graphics_draw_text("System Volume", px + 15, cy + 10, SETTINGS_TEXT_PRI);
    char vval[8]; sprintf(vval, "%d%%", g_settings.volume);
    graphics_draw_text(vval, px + pw - 50, cy + 10, SETTINGS_TEXT_SEC);
    draw_slider(px + 15, cy + 32, g_settings.volume, 100); cy += 70;

    draw_section_header(px, cy, pw, "Sound & display"); cy += 40;
    draw_card_row(px, cy, pw, "Night Light",    "Warmer screen at night", 1, &g_settings.night_light); cy += 60;
    draw_card_row(px, cy, pw, "Focus Assist",   "Minimise interruptions",  1, &g_settings.focus_assist); cy += 60;

    draw_section_header(px, cy, pw, "Power & sleep"); cy += 40;
    draw_card_row(px, cy, pw, "Screen off after 5 min",  "", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Sleep after 15 min",       "", 0, 0); cy += 60;

    draw_section_header(px, cy, pw, "About"); cy += 40;
    graphics_fill_rect(px, cy, pw, 130, SETTINGS_CARD_BG);
    graphics_draw_text("Maya OS v1.0.0",              px + 15, cy + 10, SETTINGS_TEXT_PRI);
    graphics_draw_text("Processor:  x86 (i686)",      px + 15, cy + 28, SETTINGS_TEXT_SEC);
    graphics_draw_text("RAM:        256 MB",           px + 15, cy + 44, SETTINGS_TEXT_SEC);
    graphics_draw_text("System type: 32-bit OS",       px + 15, cy + 60, SETTINGS_TEXT_SEC);
    graphics_draw_text("Build:      2026-03-07",       px + 15, cy + 76, SETTINGS_TEXT_SEC);
    graphics_draw_text("Author:     AmanNagtodeOfficial", px + 15, cy + 92, SETTINGS_TEXT_SEC);
}

static void render_bluetooth(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Bluetooth & devices"); cy += 40;
    draw_card_row(px, cy, pw, "Bluetooth",    "Toggle Bluetooth radio", 1, &g_settings.bluetooth_enabled); cy += 60;

    draw_section_header(px, cy, pw, "Paired devices"); cy += 40;
    draw_card_row(px, cy, pw, "Maya Headset Pro",  "Audio \xB7 Connected", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Maya Keyboard",     "Input \xB7 Paired",     0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Maya Mouse",        "Input \xB7 Paired",     0, 0); cy += 60;

    // Add device button
    graphics_fill_rect(px, cy, pw, 44, SETTINGS_ACCENT);
    graphics_draw_text("+ Add device", px + pw/2 - 40, cy + 14, SETTINGS_TEXT_PRI);
}

static void render_network(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Network & Internet"); cy += 40;
    draw_card_row(px, cy, pw, "Wi-Fi",        "Maya_Net - Connected",   1, &g_settings.wifi_enabled);  cy += 60;
    draw_card_row(px, cy, pw, "Airplane mode","Disable all radios",      1, &g_settings.airplane_mode); cy += 60;
    draw_card_row(px, cy, pw, "Mobile hotspot","Share internet",          1, &g_settings.hotspot_enabled); cy += 60;

    draw_section_header(px, cy, pw, "Wi-Fi networks"); cy += 40;
    draw_card_row(px, cy, pw, "Maya_Net",  "Secured \xB7 Signal: Excellent", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "HomeNet_5G","Secured \xB7 Signal: Good",       0, 0); cy += 60;
    draw_card_row(px, cy, pw, "OfficeNet", "Secured \xB7 Signal: Fair",       0, 0); cy += 60;
}

static void render_personalize(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Background & Colors"); cy += 40;
    draw_card_row(px, cy, pw, "Dark Mode",    "Use dark theme for apps and system", 1, &g_settings.dark_mode); cy += 60;
    draw_card_row(px, cy, pw, "Transparency", "Translucent surfaces",               1, &g_settings.transparency); cy += 60;
    draw_card_row(px, cy, pw, "Animations",   "Smooth window animations",           1, &g_settings.animations); cy += 60;

    draw_section_header(px, cy, pw, "Accent color"); cy += 40;
    // Swatch row
    static const uint32_t SWATCHES[] = {
        0x0078D4, 0xE81123, 0x00CC6A, 0xFF8C00,
        0x744DA9, 0xE3008C, 0x00B7C3, 0xFFB900
    };
    graphics_fill_rect(px, cy, pw, 50, SETTINGS_CARD_BG);
    for (int i = 0; i < 8; i++) {
        int sx = px + 15 + i * 36;
        graphics_fill_rect(sx, cy + 12, 26, 26, SWATCHES[i]);
        if (i == g_settings.accent_color_index) {
            graphics_draw_rect(sx - 2, cy + 10, 30, 30, SETTINGS_TEXT_PRI);
        }
    }
    cy += 55;

    draw_section_header(px, cy, pw, "Taskbar & Start"); cy += 40;
    draw_card_row(px, cy, pw, "Centered taskbar icons", "Windows 11 style", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Show search on taskbar",  "",                  0, 0); cy += 60;
}

static void render_apps(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Installed apps"); cy += 40;
    const char *installed[] = {
        "Terminal", "Notepad", "Control Panel",
        "Date & Time", "Virtual Keyboard", "File Manager",
        "Settings"
    };
    for (int i = 0; i < 7; i++) {
        graphics_fill_rect(px, cy, pw, 50, SETTINGS_CARD_BG);
        graphics_draw_text(installed[i], px + 15, cy + 18, SETTINGS_TEXT_PRI);
        graphics_draw_text("Maya OS Built-in", px + 15, cy + 34, SETTINGS_TEXT_SEC);
        graphics_draw_line(px, cy + 50, px + pw, cy + 50, SETTINGS_DIVIDER);
        cy += 56;
    }
}

static void render_accounts(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Your account"); cy += 40;
    // Avatar
    graphics_fill_rect(px, cy, pw, 80, SETTINGS_CARD_BG);
    graphics_fill_rect(px + 14, cy + 14, 52, 52, SETTINGS_ACCENT);
    char init[2] = {g_settings.user_name[0] ? g_settings.user_name[0] : 'U', '\0'};
    graphics_draw_text(init, px + 34, cy + 32, SETTINGS_TEXT_PRI);
    graphics_draw_text(g_settings.user_name,  px + 80, cy + 20, SETTINGS_TEXT_PRI);
    graphics_draw_text(g_settings.user_email, px + 80, cy + 40, SETTINGS_TEXT_SEC);
    graphics_draw_text("Administrator",         px + 80, cy + 58, SETTINGS_TEXT_SEC);
    cy += 90;

    draw_section_header(px, cy, pw, "Sign-in options"); cy += 40;
    draw_card_row(px, cy, pw, "PIN (Maya Hello)", "Quick secure login", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Password",          "Classic password",   0, 0); cy += 60;
}

static void render_time_language(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Date & Time"); cy += 40;
    draw_card_row(px, cy, pw, "Set time automatically", "Sync with internet", 1, &g_settings.auto_update); cy += 60;

    graphics_fill_rect(px, cy, pw, 60, SETTINGS_CARD_BG);
    graphics_draw_text("Current time:  2026-03-07  19:29", px + 15, cy + 20, SETTINGS_TEXT_PRI);
    graphics_draw_text("Timezone:  (UTC+05:30) India Standard Time", px + 15, cy + 38, SETTINGS_TEXT_SEC);
    cy += 66;

    draw_section_header(px, cy, pw, "Language"); cy += 40;
    draw_card_row(px, cy, pw, "System language", "English (United States)", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Region format",   "English (India)",          0, 0); cy += 60;
}

static void render_accessibility(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Vision"); cy += 40;
    draw_card_row(px, cy, pw, "Larger text",   "Scale up system font",     0, 0); cy += 60;
    draw_card_row(px, cy, pw, "High contrast", "Reduce eye strain",        0, 0); cy += 60;

    draw_section_header(px, cy, pw, "Input"); cy += 40;
    draw_card_row(px, cy, pw, "On-screen keyboard", "Touch or click to type", 0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Keyboard shortcuts",  "Enable global hotkeys",  0, 0); cy += 60;
    draw_card_row(px, cy, pw, "Sticky keys",         "Modifier key lock",       0, 0); cy += 60;

    draw_section_header(px, cy, pw, "Touch"); cy += 40;
    draw_card_row(px, cy, pw, "Touch target size", "Larger touch areas for fingers", 0, 0); cy += 60;
}

static void render_privacy(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "App permissions"); cy += 40;
    draw_card_row(px, cy, pw, "Location",   "Allow apps to access location",   1, &g_settings.location);   cy += 60;
    draw_card_row(px, cy, pw, "Camera",     "Allow apps to access camera",     1, &g_settings.camera);     cy += 60;
    draw_card_row(px, cy, pw, "Microphone", "Allow apps to access microphone", 1, &g_settings.microphone); cy += 60;

    draw_section_header(px, cy, pw, "Diagnostics & feedback"); cy += 40;
    draw_card_row(px, cy, pw, "Diagnostic data", "Send usage data to Maya",
                  1, &g_settings.diagnostic); cy += 60;
}

static void render_update(int px, int py, int pw) {
    int cy = py;
    draw_section_header(px, cy, pw, "Windows Update"); cy += 40;

    if (g_settings.update_available) {
        graphics_fill_rect(px, cy, pw, 80, 0x003D1F);
        graphics_draw_text("Update available: Maya OS v1.1.0", px + 15, cy + 15, 0x00CC6A);
        graphics_draw_text("File size: 120 MB",                px + 15, cy + 35, SETTINGS_TEXT_SEC);
        // Download button
        graphics_fill_rect(px + pw - 130, cy + 25, 110, 30, SETTINGS_ACCENT);
        graphics_draw_text("Download & install", px + pw - 125, cy + 35, SETTINGS_TEXT_PRI);
        cy += 90;
    } else {
        graphics_fill_rect(px, cy, pw, 70, SETTINGS_CARD_BG);
        graphics_draw_text("You're up to date", px + 15, cy + 15, 0x00CC6A);
        char checked[64]; sprintf(checked, "Last checked: %s", g_settings.last_checked);
        graphics_draw_text(checked,             px + 15, cy + 35, SETTINGS_TEXT_SEC);
        cy += 80;
    }

    draw_card_row(px, cy, pw, "Automatic updates", "Download and install automatically",
                  1, &g_settings.auto_update); cy += 60;
    draw_section_header(px, cy, pw, "Update history"); cy += 40;
    draw_card_row(px, cy, pw, "Maya OS v1.0.0", "2026-03-07 - Initial release", 0, 0); cy += 60;
}

// ── Sidebar ─────────────────────────────────────────────────────────────────

static void render_sidebar(int wx, int wy, int wh) {
    graphics_fill_rect(wx, wy, SIDEBAR_W, wh, SETTINGS_SIDEBAR_BG);

    // App title
    graphics_fill_rect(wx, wy, SIDEBAR_W, HEADER_H, SETTINGS_BG);
    graphics_draw_text("Settings", wx + 14, wy + 17, SETTINGS_TEXT_PRI);

    for (int i = 0; i < SEC_COUNT; i++) {
        int iy     = wy + HEADER_H + i * ITEM_H;
        uint32_t bg = (i == g_settings.active_section) ? SETTINGS_ACTIVE_BG : SETTINGS_SIDEBAR_BG;
        graphics_fill_rect(wx, iy, SIDEBAR_W, ITEM_H, bg);

        // Icon circle
        char icon_label[2] = {SECTION_ICONS[i], '\0'};
        graphics_fill_rect(wx + 12, iy + 10, 20, 20, SETTINGS_ACCENT);
        graphics_draw_text(icon_label, wx + 18, iy + 14, SETTINGS_TEXT_PRI);

        // Label
        graphics_draw_text(SECTION_LABELS[i], wx + 42, iy + 13, SETTINGS_TEXT_PRI);

        // Bottom hairline
        graphics_draw_line(wx, iy + ITEM_H - 1, wx + SIDEBAR_W, iy + ITEM_H - 1, SETTINGS_DIVIDER);
    }
}

// ── Main entry points ────────────────────────────────────────────────────────

void settings_init(void) {
    memset(&g_settings, 0, sizeof(g_settings));

    // Defaults
    g_settings.dark_mode        = 1;
    g_settings.transparency     = 1;
    g_settings.animations       = 1;
    g_settings.accent_color_index = 0;
    g_settings.volume           = 75;
    g_settings.brightness       = 80;
    g_settings.wifi_enabled     = 1;
    g_settings.bluetooth_enabled= 1;
    g_settings.auto_update      = 1;
    g_settings.diagnostic       = 1;
    g_settings.update_available = 0;

    strncpy(g_settings.user_name,  "User", 31);
    strncpy(g_settings.user_email, "user@mayaos.local", 63);
    strncpy(g_settings.last_checked,"2026-03-07 19:29", 31);

    g_settings.win = window_create("Settings", 60, 40, 740, 500);
}

void settings_draw(void) {
    if (!g_settings.win) return;

    window_t *w = g_settings.win;
    int wx = w->x, wy = w->y, wh = w->height;

    // Background
    graphics_fill_rect(wx, wy, w->width, wh, SETTINGS_BG);

    render_sidebar(wx, wy + 22 /* after title bar */, wh - 22);

    // Content area
    int px = wx + SIDEBAR_W + PANEL_PAD;
    int py = wy + 22 + HEADER_H + 8;
    int pw = w->width - SIDEBAR_W - PANEL_PAD * 2;

    // Section title
    graphics_draw_text(SECTION_LABELS[g_settings.active_section],
                       px, py - 24, SETTINGS_TEXT_PRI);
    graphics_draw_line(px, py - 6, px + pw, py - 6, SETTINGS_DIVIDER);

    switch (g_settings.active_section) {
        case SEC_HOME:          render_home(px, py, pw);          break;
        case SEC_SYSTEM:        render_system(px, py, pw);        break;
        case SEC_BLUETOOTH:     render_bluetooth(px, py, pw);     break;
        case SEC_NETWORK:       render_network(px, py, pw);       break;
        case SEC_PERSONALIZE:   render_personalize(px, py, pw);   break;
        case SEC_APPS:          render_apps(px, py, pw);          break;
        case SEC_ACCOUNTS:      render_accounts(px, py, pw);      break;
        case SEC_TIME_LANGUAGE: render_time_language(px, py, pw); break;
        case SEC_ACCESSIBILITY: render_accessibility(px, py, pw); break;
        case SEC_PRIVACY:       render_privacy(px, py, pw);       break;
        case SEC_UPDATE:        render_update(px, py, pw);        break;
    }

    window_render(g_settings.win);
}

// Click handling: sidebar item selection
void settings_handle_mouse(int x, int y, uint8_t buttons) {
    if (!g_settings.win || !buttons) return;
    window_t *w = g_settings.win;
    int wx = w->x, wy = w->y + 22;

    // Check sidebar items
    if (x >= wx && x <= wx + SIDEBAR_W) {
        int rel_y = y - wy - HEADER_H;
        if (rel_y >= 0) {
            int idx = rel_y / ITEM_H;
            if (idx >= 0 && idx < SEC_COUNT) {
                g_settings.active_section = idx;
            }
        }
    }
}

void settings_handle_key(char ascii) {
    (void)ascii; // Future: keyboard nav
}
