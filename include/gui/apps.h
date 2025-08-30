#ifndef MAYA_APPS_H
#define MAYA_APPS_H

#include <stdint.h> // Include for standard integer types
#include "include/gui/window.h"
#include "include/gui/input.h" // Include for maya_input_event_t

// Application Categories
#define CATEGORY_ACCESSORIES "Accessories"
#define CATEGORY_INTERNET "Internet"
#define CATEGORY_OFFICE "Office"
#define CATEGORY_GRAPHICS "Graphics"
#define CATEGORY_SOUND_VIDEO "Sound & Video"
#define CATEGORY_SYSTEM "System Tools"
#define CATEGORY_GAMES "Games"
#define CATEGORY_DEVELOPMENT "Development"

// Built-in Application IDs
#define APP_FILES 0
#define APP_TERMINAL 1
#define APP_TEXT_EDITOR 2
#define APP_CALCULATOR 3
#define APP_SETTINGS 4
#define APP_WEB_BROWSER 5
#define APP_IMAGE_VIEWER 6
#define APP_MUSIC_PLAYER 7
#define APP_VIDEO_PLAYER 8
#define APP_SOFTWARE_CENTER 9

// Maya Application Window Structure
typedef struct {
    window_t* window;
    uint8_t app_id;
    char title[128];
    uint8_t minimized;
    uint8_t maximized;
    uint8_t focused;
    void* app_data; // Maya App-specific data
} maya_app_window_t;

// Maya Application Function Pointers
typedef struct {
    void (*init)(maya_app_window_t* app_window);
    void (*render)(maya_app_window_t* app_window);
    void (*handle_input)(maya_app_window_t* app_window, maya_input_event_t* event);
    void (*update)(maya_app_window_t* app_window);
    void (*cleanup)(maya_app_window_t* app_window);
} maya_app_interface_t;

// Function Declarations
void maya_apps_init(void);
maya_app_window_t* maya_app_create(uint8_t app_id, int x, int y, int width, int height);
void maya_app_destroy(maya_app_window_t* app_window);
void maya_app_focus(maya_app_window_t* app_window);
void maya_app_minimize(maya_app_window_t* app_window);
void maya_app_maximize(maya_app_window_t* app_window);
void maya_app_restore(maya_app_window_t* app_window);

// Built-in Maya Applications
void maya_files_init(maya_app_window_t* app_window);
void maya_files_render(maya_app_window_t* app_window);
void maya_files_handle_input(maya_app_window_t* app_window, maya_input_event_t* event);

void maya_terminal_init(maya_app_window_t* app_window);
void maya_terminal_render(maya_app_window_t* app_window);
void maya_terminal_handle_input(maya_app_window_t* app_window, maya_input_event_t* event);

void maya_text_editor_init(maya_app_window_t* app_window);
void maya_text_editor_render(maya_app_window_t* app_window);
void maya_text_editor_handle_input(maya_app_window_t* app_window, maya_input_event_t* event);

void maya_calculator_init(maya_app_window_t* app_window);
void maya_calculator_render(maya_app_window_t* app_window);
void maya_calculator_handle_input(maya_app_window_t* app_window, maya_input_event_t* event);

void maya_settings_init(maya_app_window_t* app_window);
void maya_settings_render(maya_app_window_t* app_window);
void maya_settings_handle_input(maya_app_window_t* app_window, maya_input_event_t* event);

// Maya Application Data Structures
typedef struct {
    char current_path[512]; // Current directory path for file manager
    char* file_list[256];   // List of files in the current directory
    int file_count;         // Number of files in the list
    int selected_file;      // Index of the currently selected file
    int scroll_offset;      // Scroll offset for displaying the file list
} files_app_data_t;

typedef struct {
    char command_history[64][256]; // Command history buffer
    int history_count;             // Number of commands in history
    int history_pos;               // Current position in command history
    char current_line[256];        // Current command line input
    int cursor_pos;                // Cursor position in the current line
    char output_buffer[4096];      // Buffer for terminal output
    int output_length;             // Length of the output buffer content
} terminal_app_data_t;

typedef struct {
    char* text_buffer;     // Buffer for text editor content
    int buffer_size;       // Size of the text buffer
    int cursor_pos;        // Cursor position in the text buffer
    int scroll_line;       // Scroll offset for displaying text
    uint8_t modified;      // Flag indicating if the file has been modified
    char filename[256];    // Filename of the loaded file
} text_editor_app_data_t;

typedef struct {
    char display[32];    // Calculator display string
    double value;        // Current value in the calculator
    double memory;       // Memory value in the calculator
    char operator;       // Current operator (+, -, *, /)
    uint8_t new_number;  // Flag indicating if the next digit starts a new number
    uint8_t error;       // Flag indicating a calculator error
} calculator_app_data_t;

typedef struct {
    int selected_category; // Selected application category in settings
    int selected_item;     // Selected item within the category
    uint8_t wallpaper_index; // Index of the selected wallpaper
    uint8_t theme_index;     // Index of the selected theme
    uint8_t show_launcher;   // Flag to show/hide the launcher
    uint8_t show_taskbar;    // Flag to show/hide the taskbar
} settings_app_data_t;

