#ifndef UBUNTU_APPS_H
#define UBUNTU_APPS_H

#include <stdint.h>
#include "gui/window.h"

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

// Application Window Structure
typedef struct {
    window_t* window;
        uint8_t app_id;
            char title[128];
                uint8_t minimized;
                    uint8_t maximized;
                        uint8_t focused;
                            void* app_data; // App-specific data
                            } ubuntu_app_window_t;

                            // Application Function Pointers
                            typedef struct {
                                void (*init)(ubuntu_app_window_t* app_window);
                                    void (*render)(ubuntu_app_window_t* app_window);
                                        void (*handle_input)(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);
                                            void (*update)(ubuntu_app_window_t* app_window);
                                                void (*cleanup)(ubuntu_app_window_t* app_window);
                                                } ubuntu_app_interface_t;

                                                // Function Declarations
                                                void ubuntu_apps_init(void);
                                                ubuntu_app_window_t* ubuntu_app_create(uint8_t app_id, int x, int y, int width, int height);
                                                void ubuntu_app_destroy(ubuntu_app_window_t* app_window);
                                                void ubuntu_app_focus(ubuntu_app_window_t* app_window);
                                                void ubuntu_app_minimize(ubuntu_app_window_t* app_window);
                                                void ubuntu_app_maximize(ubuntu_app_window_t* app_window);
                                                void ubuntu_app_restore(ubuntu_app_window_t* app_window);

                                                // Built-in Applications
                                                void ubuntu_files_init(ubuntu_app_window_t* app_window);
                                                void ubuntu_files_render(ubuntu_app_window_t* app_window);
                                                void ubuntu_files_handle_input(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);

                                                void ubuntu_terminal_init(ubuntu_app_window_t* app_window);
                                                void ubuntu_terminal_render(ubuntu_app_window_t* app_window);
                                                void ubuntu_terminal_handle_input(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);

                                                void ubuntu_text_editor_init(ubuntu_app_window_t* app_window);
                                                void ubuntu_text_editor_render(ubuntu_app_window_t* app_window);
                                                void ubuntu_text_editor_handle_input(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);

                                                void ubuntu_calculator_init(ubuntu_app_window_t* app_window);
                                                void ubuntu_calculator_render(ubuntu_app_window_t* app_window);
                                                void ubuntu_calculator_handle_input(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);

                                                void ubuntu_settings_init(ubuntu_app_window_t* app_window);
                                                void ubuntu_settings_render(ubuntu_app_window_t* app_window);
                                                void ubuntu_settings_handle_input(ubuntu_app_window_t* app_window, ubuntu_input_event_t* event);

                                                // Application Data Structures
                                                typedef struct {
                                                    char current_path[512];
                                                        char* file_list[256];
                                                            int file_count;
                                                                int selected_file;
                                                                    int scroll_offset;
                                                                    } files_app_data_t;

                                                                    typedef struct {
                                                                        char command_history[64][256];
                                                                            int history_count;
                                                                                int history_pos;
                                                                                    char current_line[256];
                                                                                        int cursor_pos;
                                                                                            char output_buffer[4096];
                                                                                                int output_length;
                                                                                                } terminal_app_data_t;

                                                                                                typedef struct {
                                                                                                    char* text_buffer;
                                                                                                        int buffer_size;
                                                                                                            int cursor_pos;
                                                                                                                int scroll_line;
                                                                                                                    uint8_t modified;
                                                                                                                        char filename[256];
                                                                                                                        } text_editor_app_data_t;

                                                                                                                        typedef struct {
                                                                                                                            char display[32];
                                                                                                                                double value;
                                                                                                                                    double memory;
                                                                                                                                        char operator;
                                                                                                                                            uint8_t new_number;
                                                                                                                                                uint8_t error;
                                                                                                                                                } calculator_app_data_t;

                                                                                                                                                typedef struct {
                                                                                                                                                    int selected_category;
                                                                                                                                                        int selected_item;
                                                                                                                                                            uint8_t wallpaper_index;
                                                                                                                                                                uint8_t theme_index;
                                                                                                                                                                    uint8_t show_launcher;
                                                                                                                                                                        uint8_t show_taskbar;
                                                                                                                                                                        } settings_app_data_t;

                                                                                                                                                                        #endif
