
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdint.h>
#include "gui/window.h"

#define MAX_FILES_PER_DIR 256
#define MAX_PATH_LEN 512
#define SIDEBAR_WIDTH 200

typedef enum {
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_REGULAR,
    FILE_TYPE_EXECUTABLE,
    FILE_TYPE_SYMLINK
} file_type_t;

typedef struct {
    char name[128];
    char full_path[MAX_PATH_LEN];
    file_type_t type;
    uint32_t size;
    uint32_t modified_time;
    uint8_t selected;
    uint8_t *icon_data;
} file_entry_t;

typedef struct {
    char current_path[MAX_PATH_LEN];
    file_entry_t files[MAX_FILES_PER_DIR];
    int file_count;
    int selected_file;
    int view_mode; // 0=list, 1=icons
    uint8_t show_hidden;
} file_browser_t;

typedef struct {
    window_t *window;
    file_browser_t browser;
    uint8_t sidebar_visible;
} file_manager_t;

// File manager functions
file_manager_t *file_manager_create(void);
void file_manager_destroy(file_manager_t *fm);
void file_manager_open_directory(file_manager_t *fm, const char *path);
void file_manager_refresh(file_manager_t *fm);
void file_manager_handle_click(file_manager_t *fm, int x, int y);
void file_manager_handle_key(file_manager_t *fm, char key);
void file_manager_render(file_manager_t *fm);

#endif


#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdint.h>
#include "gui/window.h"

#define MAX_FILES_PER_DIR 256
#define MAX_PATH_LEN 512
#define SIDEBAR_WIDTH 200

typedef enum {
    FILE_TYPE_DIRECTORY,
        FILE_TYPE_REGULAR,
            FILE_TYPE_EXECUTABLE,
                FILE_TYPE_SYMLINK
                } file_type_t;

                typedef struct {
                    char name[128];
                        char full_path[MAX_PATH_LEN];
                            file_type_t type;
                                uint32_t size;
                                    uint32_t modified_time;
                                        uint8_t selected;
                                            uint8_t *icon_data;
                                            } file_entry_t;

                                            typedef struct {
                                                char current_path[MAX_PATH_LEN];
                                                    file_entry_t files[MAX_FILES_PER_DIR];
                                                        int file_count;
                                                            int selected_file;
                                                                int view_mode; // 0=list, 1=icons
                                                                    uint8_t show_hidden;
                                                                    } file_browser_t;

                                                                    typedef struct {
                                                                        window_t *window;
                                                                            file_browser_t browser;
                                                                                uint8_t sidebar_visible;
                                                                                } file_manager_t;

                                                                                // File manager functions
                                                                                file_manager_t *file_manager_create(void);
                                                                                void file_manager_destroy(file_manager_t *fm);
                                                                                void file_manager_open_directory(file_manager_t *fm, const char *path);
                                                                                void file_manager_refresh(file_manager_t *fm);
                                                                                void file_manager_handle_click(file_manager_t *fm, int x, int y);
                                                                                void file_manager_handle_key(file_manager_t *fm, char key);
                                                                                void file_manager_render(file_manager_t *fm);

                                                                                #endif
