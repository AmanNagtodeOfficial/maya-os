
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <stdint.h>

#define LAUNCHER_WIDTH 64
#define MAX_LAUNCHER_ITEMS 16
#define LAUNCHER_ITEM_SIZE 48
#define LAUNCHER_PADDING 8

typedef struct {
    char *name;
        char *command;
            uint8_t *icon_data;
                uint8_t pinned;
                    uint8_t running;
                        uint8_t highlighted;
                        } launcher_item_t;

                        typedef struct {
                            launcher_item_t items[MAX_LAUNCHER_ITEMS];
                                int item_count;
                                    int scroll_offset;
                                        uint8_t auto_hide;
                                            uint8_t visible;
                                            } launcher_t;

                                            // Launcher functions
                                            void launcher_init(void);
                                            void launcher_render(void);
                                            void launcher_add_item(const char *name, const char *command, uint8_t *icon);
                                            void launcher_remove_item(int index);
                                            void launcher_handle_click(int x, int y);
                                            void launcher_set_auto_hide(uint8_t enabled);

                                            #endif
