#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/widgets.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"

#define TERM_ROWS 20
#define TERM_COLS 80
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

static window_t* term_window = NULL;
static char* term_buffer = NULL;
static char current_command[256];
static int cmd_pos = 0;
static int cursor_row = 0;
static int cursor_col = 0;

void terminal_writeln(const char* text) {
    // Basic terminal scrolling skip for now
    graphics_draw_text(text, term_window->x + 5, term_window->y + WINDOW_TITLE_HEIGHT + (cursor_row * CHAR_HEIGHT) + 5, 0x00FF00);
    cursor_row++;
    cursor_col = 0;
}

void terminal_handle_input(uint32_t key) {
    if (!term_window || !term_window->visible) return;

    if (key == '\n') {
        current_command[cmd_pos] = '\0';
        terminal_writeln(current_command);
        
        // Placeholder for command execution
        if (strcmp(current_command, "help") == 0) {
            terminal_writeln("Available commands: help, clear, exit");
        } else if (strcmp(current_command, "clear") == 0) {
            window_clear(term_window, 0x000000);
            cursor_row = 0;
            cursor_col = 0;
        } else if (strcmp(current_command, "exit") == 0) {
            window_hide(term_window);
        } else {
            terminal_writeln("Unknown command");
        }
        
        cmd_pos = 0;
        memset(current_command, 0, sizeof(current_command));
    } else if (key == '\b') {
        if (cmd_pos > 0) {
            cmd_pos--;
            current_command[cmd_pos] = '\0';
        }
    } else if (cmd_pos < sizeof(current_command) - 1) {
        current_command[cmd_pos++] = (char)key;
    }

    // Refresh display
    window_clear(term_window, 0x000000);
    // Redraw all lines (simplified: just current command for now)
    char prompt[300];
    sprintf(prompt, "> %s", current_command);
    graphics_draw_text(prompt, term_window->x + 5, term_window->y + WINDOW_TITLE_HEIGHT + 5, 0x00FF00);
    window_refresh(term_window);
}

void terminal_init(void) {
    term_window = window_create("Terminal", 100, 100, 640, 480);
    if (!term_window) return;

    window_clear(term_window, 0x000000); // Black background
    terminal_writeln("Maya OS Terminal v1.0");
    terminal_writeln("Type 'help' for commands.");
    window_refresh(term_window);
}

void terminal_draw(void) {
    if (term_window && term_window->visible) {
        window_render(term_window);
    }
}
