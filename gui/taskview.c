/**
 * Maya OS - Multitasking & Task View
 * Alt+Tab / Win+Tab style task switcher + virtual desktops
 * Author: AmanNagtodeOfficial
 */

#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/input.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"

// ── Colors ───────────────────────────────────────────────────────────────────
#define TV_OVERLAY_BG   0xCC000000  // semi-transparent black (alpha-style, rendered as nearly-black)
#define TV_CARD_BG      0x1E1E1E
#define TV_CARD_ACTIVE  0x0078D4
#define TV_CARD_HOVER   0x333333
#define TV_TEXT         0xFFFFFF
#define TV_TEXT_DIM     0x888888
#define TV_BADGE_BG     0x444444

// ── Task card dimensions ─────────────────────────────────────────────────────
#define TV_CARD_W       180
#define TV_CARD_H       120
#define TV_CARD_GAP     20
#define TV_THUMB_H      80
#define TV_TITLE_H      30
#define TV_V_PADDING    80   // distance from top of screen

#define MAX_VIRTUAL_DESKTOPS 4

typedef struct {
    uint8_t  visible;           // Is Task View open?
    int      focused_idx;       // Which card is highlighted by keyboard
    int      virtual_desk;      // Active virtual desktop (0..3)
    int      desk_count;        // How many desktops exist
    uint8_t  alt_tab_mode;      // Compact Alt+Tab instead of full Win+Tab
} task_view_t;

static task_view_t tv = {0};

// ── Public API ───────────────────────────────────────────────────────────────

void taskview_open(uint8_t alt_tab) {
    tv.visible      = 1;
    tv.alt_tab_mode = alt_tab;
    tv.focused_idx  = 0;
    if (tv.desk_count < 1) tv.desk_count = 1;
}

void taskview_close(void) {
    tv.visible = 0;
}

uint8_t taskview_is_open(void) {
    return tv.visible;
}

// ── Drawing ───────────────────────────────────────────────────────────────────
/*
 * Draws a single window card preview at position (x,y).
 * It renders the title-bar colour of the real window as the "screenshot".
 */
static void draw_window_card(int x, int y, window_t *w, uint8_t active) {
    // Card background
    uint32_t bg = active ? TV_CARD_ACTIVE : TV_CARD_BG;
    graphics_fill_rect(x, y, TV_CARD_W, TV_CARD_H, bg);

    // Thumbnail area – approximation using the window's title-bar colour
    graphics_fill_rect(x + 4, y + 4, TV_CARD_W - 8, TV_THUMB_H, 0x2C2C2C);

    // Mini title bar drawn inside the thumbnail
    graphics_fill_rect(x + 4, y + 4, TV_CARD_W - 8, 14, 0x202020);
    char short_title[14];
    strncpy(short_title, w->title, 13);
    short_title[13] = '\0';
    graphics_draw_text(short_title, x + 8, y + 6, 0xCCCCCC);

    // App name label below thumbnail
    graphics_draw_text(w->title, x + 6, y + TV_THUMB_H + 10, TV_TEXT);

    // Active underline
    if (active) {
        graphics_fill_rect(x, y + TV_CARD_H - 3, TV_CARD_W, 3, 0x00AAFF);
    }

    // Border
    graphics_draw_rect(x, y, TV_CARD_W, TV_CARD_H,
                       active ? 0x0091EA : 0x555555);
}

/*
 * Full Task View (Win + Tab) – shows virtual desktop strip + window thumbnails.
 */
static void draw_task_view_full(int sw, int sh) {
    // Dim overlay (approximated as a dark fill)
    graphics_fill_rect(0, 0, sw, sh, 0x0A0A0A);

    extern uint32_t window_get_count(void);
    extern window_t *window_get_by_index(int);
    int wc = (int)window_get_count();

    // ── Virtual desktop strip at bottom ──────────────────────────────────────
    int desk_strip_h = 70;
    int desk_y       = sh - desk_strip_h;
    graphics_fill_rect(0, desk_y, sw, desk_strip_h, 0x141414);
    graphics_draw_line(0, desk_y, sw, desk_y, 0x444444);

    graphics_draw_text("Virtual desktops", 20, desk_y + 6, TV_TEXT_DIM);

    for (int d = 0; d < tv.desk_count; d++) {
        int dx = 20 + d * 120;
        uint32_t dc = (d == tv.virtual_desk) ? TV_CARD_ACTIVE : TV_CARD_BG;
        graphics_fill_rect(dx, desk_y + 22, 100, 38, dc);
        char dlabel[16];
        sprintf(dlabel, "Desktop %d", d + 1);
        graphics_draw_text(dlabel, dx + 10, desk_y + 36, TV_TEXT);
    }

    // "+ New Desktop" button
    int nd_x = 20 + tv.desk_count * 120;
    if (nd_x + 110 < sw - 30) {
        graphics_fill_rect(nd_x, desk_y + 22, 110, 38, 0x333333);
        graphics_draw_text("+ New Desktop", nd_x + 10, desk_y + 36, TV_TEXT);
    }

    // ── Window cards ─────────────────────────────────────────────────────────
    if (wc == 0) {
        graphics_draw_text("No open windows on this desktop",
                           sw / 2 - 100, sh / 2, TV_TEXT_DIM);
        return;
    }

    int total_w = wc * TV_CARD_W + (wc - 1) * TV_CARD_GAP;
    int start_x = (sw - total_w) / 2;
    int card_y  = TV_V_PADDING;

    for (int i = 0; i < wc; i++) {
        window_t *w = window_get_by_index(i);
        if (!w) continue;
        int cx = start_x + i * (TV_CARD_W + TV_CARD_GAP);
        draw_window_card(cx, card_y, w, (i == tv.focused_idx));
    }

    // Hint bar
    graphics_draw_text("Arrow keys: navigate   Enter/click: switch   Esc: close", 30, sh - desk_strip_h - 20, TV_TEXT_DIM);
}

/*
 * Compact Alt+Tab strip (similar to Windows Alt+Tab).
 */
static void draw_alt_tab(int sw, int sh) {
    extern uint32_t window_get_count(void);
    extern window_t *window_get_by_index(int);
    int wc = (int)window_get_count();
    if (wc == 0) return;

    int strip_h = 100;
    int strip_w = wc * (TV_CARD_W / 2 + 10) + 20;
    if (strip_w > sw - 60) strip_w = sw - 60;

    int sx = (sw - strip_w) / 2;
    int sy = (sh - strip_h) / 2;

    graphics_fill_rect(sx, sy, strip_w, strip_h, 0x1A1A1A);
    graphics_draw_rect(sx, sy, strip_w, strip_h, 0x555555);

    int thumb_w = TV_CARD_W / 2;
    int thumb_h = strip_h - 24;

    for (int i = 0; i < wc; i++) {
        window_t *w = window_get_by_index(i);
        if (!w) continue;
        int cx = sx + 8 + i * (thumb_w + 8);
        if (cx + thumb_w > sx + strip_w - 8) break;

        uint32_t bg = (i == tv.focused_idx) ? TV_CARD_ACTIVE : TV_CARD_BG;
        graphics_fill_rect(cx, sy + 6, thumb_w, thumb_h, bg);

        char label[8];
        label[0] = w->title[0]; label[1] = '\0';
        graphics_draw_text(label, cx + thumb_w / 2 - 3, sy + 8, TV_TEXT);

        if (i == tv.focused_idx) {
            graphics_draw_rect(cx - 1, sy + 5, thumb_w + 2, thumb_h + 2, 0x0091EA);
        }
    }

    // Title of focused window
    window_t *fw = window_get_by_index(tv.focused_idx);
    if (fw) {
        graphics_draw_text(fw->title, sx + 10, sy + strip_h - 16, TV_TEXT);
    }
}

void taskview_draw(int screen_w, int screen_h) {
    if (!tv.visible) return;
    if (tv.alt_tab_mode) {
        draw_alt_tab(screen_w, screen_h);
    } else {
        draw_task_view_full(screen_w, screen_h);
    }
}

// ── Input handling ────────────────────────────────────────────────────────────

void taskview_handle_key(char ascii, uint8_t scancode) {
    if (!tv.visible) return;

    extern uint32_t window_get_count(void);
    extern window_t *window_get_by_index(int);
    extern void window_focus(window_t *w);
    int wc = (int)window_get_count();

    if (scancode == 77 /* RIGHT */ || ascii == '\t') {
        tv.focused_idx = (tv.focused_idx + 1) % (wc ? wc : 1);
    } else if (scancode == 75 /* LEFT */) {
        tv.focused_idx = (tv.focused_idx - 1 + wc) % (wc ? wc : 1);
    } else if (ascii == '\r' || ascii == '\n') {
        // Focus the selected window
        window_t *w = window_get_by_index(tv.focused_idx);
        if (w) window_focus(w);
        taskview_close();
    } else if (ascii == 27 /* ESC */) {
        taskview_close();
    }
}

void taskview_handle_mouse(int x, int y, uint8_t buttons, int sw, int sh) {
    if (!tv.visible || !buttons) return;

    extern uint32_t window_get_count(void);
    extern window_t *window_get_by_index(int);
    extern void window_focus(window_t *w);
    int wc = (int)window_get_count();

    if (tv.alt_tab_mode) {
        taskview_close();
        return;
    }

    // Virtual desktop strip
    int desk_y = sh - 70;
    if (y >= desk_y) {
        // Check + New Desktop
        int nd_x = 20 + tv.desk_count * 120;
        if (x >= nd_x && x <= nd_x + 110 && tv.desk_count < MAX_VIRTUAL_DESKTOPS) {
            tv.desk_count++;
        }
        // Check existing desktops
        for (int d = 0; d < tv.desk_count; d++) {
            int dx = 20 + d * 120;
            if (x >= dx && x <= dx + 100) {
                tv.virtual_desk = d;
            }
        }
        return;
    }

    // Window cards
    int total_w = wc * TV_CARD_W + (wc - 1) * TV_CARD_GAP;
    int start_x = (sw - total_w) / 2;
    for (int i = 0; i < wc; i++) {
        int cx = start_x + i * (TV_CARD_W + TV_CARD_GAP);
        int cy = TV_V_PADDING;
        if (x >= cx && x <= cx + TV_CARD_W && y >= cy && y <= cy + TV_CARD_H) {
            window_t *w = window_get_by_index(i);
            if (w) window_focus(w);
            taskview_close();
            return;
        }
    }

    // Click outside → close
    taskview_close();
}
