
#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "gui/window.h"
#include "gui/graphics.h"

#define MAX_DESKTOP_ICONS 32
#define TASKBAR_HEIGHT 28
#define LAUNCHER_WIDTH 64

typedef struct {
    int x, y;
        int width, height;
            char *name;
                char *path;
                    uint8_t *icon_data;
                        uint8_t selected;
                        } desktop_icon_t;

                        typedef struct {
                            desktop_icon_t icons[MAX_DESKTOP_ICONS];
                                int icon_count;
                                    uint8_t *wallpaper;
                                        uint8_t show_launcher;
                                            uint8_t show_taskbar;
                                            } desktop_t;

                                            // Desktop functions
                                            void desktop_init(void);
                                            void desktop_render(void);
                                            void desktop_add_icon(const char *name, const char *path, int x, int y);
                                            void desktop_handle_click(int x, int y);
                                            void desktop_set_wallpaper(uint8_t *wallpaper_data);

                                            #endif








#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "gui/window.h"
#include "gui/graphics.h"

#define MAX_DESKTOP_ICONS 32
#define TASKBAR_HEIGHT 28
#define LAUNCHER_WIDTH 64

typedef struct {
    int x, y;
        int width, height;
            char *name;
                char *path;
                    uint8_t *icon_data;
                        uint8_t selected;
                        } desktop_icon_t;

                        typedef struct {
                            desktop_icon_t icons[MAX_DESKTOP_ICONS];
                                int icon_count;
                                    uint8_t *wallpaper;
                                        uint8_t show_launcher;
                                            uint8_t show_taskbar;
                                            } desktop_t;

                                            // Desktop functions
                                            void desktop_init(void);
                                            void desktop_render(void);
                                            void desktop_add_icon(const char *name, const char *path, int x, int y);
                                            void desktop_handle_click(int x, int y);
                                            void desktop_set_wallpaper(uint8_t *wallpaper_data);

                                            #endif
