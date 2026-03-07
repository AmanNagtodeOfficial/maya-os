#include "gui/apps.h"
#include "gui/window.h"
#include "gui/graphics.h"
#include "gui/widgets.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "apps/shell.h"

#define TERM_ROWS 25
#define TERM_COLS 80
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

static window_t* term_window = NULL;
static char current_command[256];
static int cmd_pos = 0;
static int cursor_row = 0;
static char term_lines[TERM_ROWS][TERM_COLS];
static int line_count = 0;

void terminal_writeln(const char* text) {
    if (line_count < TERM_ROWS) {
        strncpy(term_lines[line_count], text, TERM_COLS - 1);
        line_count++;
    } else {
        // Scroll: shift lines up
        for (int i = 0; i < TERM_ROWS - 1; i++) {
            memcpy(term_lines[i], term_lines[i+1], TERM_COLS);
        }
        strncpy(term_lines[TERM_ROWS-1], text, TERM_COLS - 1);
    }
}

static void terminal_shell_output(const char* text) {
    // Break into lines if needed
    terminal_writeln(text);
}

void terminal_handle_input(uint32_t key) {
    if (!term_window || !term_window->visible) return;

    if (key == '\n') {
        current_command[cmd_pos] = '\0';
        char prompt[300];
        sprintf(prompt, "> %s", current_command);
        terminal_writeln(prompt);
        
        shell_execute(current_command);
        
        cmd_pos = 0;
        memset(current_command, 0, sizeof(current_command));
    } else if (key == '\b') {
        if (cmd_pos > 0) {
            current_command[--cmd_pos] = '\0';
        }
    } else if (key == '\t') { // Tab for autocomplete
        char result[256] = {0};
        shell_autocomplete(current_command, result);
        if (strlen(result) > 0) {
            strcpy(current_command, result);
            cmd_pos = strlen(result);
        }
    } else if (cmd_pos < sizeof(current_command) - 1 && key < 128) {
        current_command[cmd_pos++] = (char)key;
    }

    // Refresh display
    window_clear(term_window, 0x000000);
    for (int i = 0; i < line_count; i++) {
        graphics_draw_text(term_lines[i], term_window->x + 5, term_window->y + WINDOW_TITLE_HEIGHT + (i * CHAR_HEIGHT) + 5, 0x00FF00);
    }
    
    // Draw current prompt
    char prompt[300];
    sprintf(prompt, "> %s_", current_command);
    graphics_draw_text(prompt, term_window->x + 5, term_window->y + WINDOW_TITLE_HEIGHT + (line_count * CHAR_HEIGHT) + 5, 0x00FF00);
    
    window_refresh(term_window);
}

#include "gui/notification.h"

void terminal_init(void) {
    notification_push("Terminal", "Command Line Interface active.");
    term_window = window_create("Terminal", 100, 100, 640, 480);
    if (!term_window) return;

    memset(term_lines, 0, sizeof(term_lines));
    line_count = 0;

    shell_init(terminal_shell_output);
    
    terminal_writeln("Maya OS Terminal v1.1");
    terminal_writeln("Type 'help' for commands.");
    window_refresh(term_window);
}

void terminal_draw(void) {
    if (term_window && term_window->visible) {
        window_render(term_window);
    }
}

