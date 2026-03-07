#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/widgets.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define NOTEPAD_MAX_LINES 100
#define NOTEPAD_MAX_COLS 80
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

static window_t* notepad_window = NULL;
static char text_buffer[NOTEPAD_MAX_LINES][NOTEPAD_MAX_COLS];
static int cursor_row = 0;
static int cursor_col = 0;
static int max_written_row = 0;

void notepad_draw_content(void) {
    if (!notepad_window || !notepad_window->visible) return;

    // Clear window background (white)
    window_draw_rect(notepad_window, 0, 0, notepad_window->width - 4, notepad_window->height - WINDOW_TITLE_HEIGHT - 4, 0xFFFFFF);

    // Menu Bar Placeholder (Light Grey)
    window_draw_rect(notepad_window, 0, 0, notepad_window->width - 4, 20, 0xF0F0F0);
    graphics_draw_text("File   Edit   Format   View   Help", notepad_window->x + 10, notepad_window->y + WINDOW_TITLE_HEIGHT + 6, 0x000000);
    graphics_draw_line(notepad_window->x + 2, notepad_window->y + WINDOW_TITLE_HEIGHT + 20, notepad_window->x + notepad_window->width - 4, notepad_window->y + WINDOW_TITLE_HEIGHT + 20, 0xCCCCCC);

    // Draw text lines
    int start_y = notepad_window->y + WINDOW_TITLE_HEIGHT + 25;
    for (int r = 0; r <= max_written_row; r++) {
        if (strlen(text_buffer[r]) > 0) {
            graphics_draw_text(text_buffer[r], notepad_window->x + 5, start_y + (r * CHAR_HEIGHT), 0x000000);
        }
    }

    // Draw Cursor
    graphics_fill_rect(notepad_window->x + 5 + (cursor_col * CHAR_WIDTH), start_y + (cursor_row * CHAR_HEIGHT), 2, CHAR_HEIGHT, 0x000000);

    window_refresh(notepad_window);
}

void notepad_handle_input(uint32_t key) {
    if (!notepad_window || !notepad_window->visible) return;

    if (key == '\n') {
        if (cursor_row < NOTEPAD_MAX_LINES - 1) {
            cursor_row++;
            cursor_col = 0;
            if (cursor_row > max_written_row) max_written_row = cursor_row;
        }
    } else if (key == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            text_buffer[cursor_row][cursor_col] = '\0';
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = strlen(text_buffer[cursor_row]);
        }
    } else if (key >= 32 && key <= 126) { // Printable characters
        if (cursor_col < NOTEPAD_MAX_COLS - 1) {
            text_buffer[cursor_row][cursor_col++] = (char)key;
            text_buffer[cursor_row][cursor_col] = '\0';
        }
    }

    notepad_draw_content();
}

void notepad_init(void) {
    // Fluent Window decorations will automatically apply via window_create
    notepad_window = window_create("Untitled - Notepad", 150, 100, 500, 400);
    if (!notepad_window) return;

    // Initialize buffer
    for (int i = 0; i < NOTEPAD_MAX_LINES; i++) {
        memset(text_buffer[i], 0, NOTEPAD_MAX_COLS);
    }
    cursor_row = 0;
    cursor_col = 0;
    max_written_row = 0;

    notepad_draw_content();
}

void notepad_save(const char* filename) {
    vfs_node_t* node = vfs_open(filename, 0); // Simplified open
    if (node) {
        for (int i = 0; i <= max_written_row; i++) {
            vfs_write(node, 0, strlen(text_buffer[i]), text_buffer[i]);
            vfs_write(node, 0, 1, "\n");
        }
        vfs_close(node);
    }
}

void notepad_load(const char* filename) {
    vfs_node_t* node = vfs_open(filename, 0);
    if (node) {
        // Simplified load logic
        vfs_close(node);
    }
}

void notepad_draw(void) {
    if (notepad_window && notepad_window->visible) {
        window_render(notepad_window);
    }
}
