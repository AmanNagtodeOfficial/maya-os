
#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>
#include "kernel/timer.h"

#define TASKBAR_HEIGHT 28
#define SYSTEM_TRAY_WIDTH 200

typedef struct {
    char time_str[16];
        char date_str[32];
            uint8_t battery_level;
                uint8_t wifi_strength;
                    uint8_t volume_level;
                    } system_status_t;

                    typedef struct {
                        int x, y, width, height;
                            system_status_t status;
                                uint8_t visible;
                                } taskbar_t;

                                // Taskbar functions
                                void taskbar_init(void);
                                void taskbar_render(void);
                                void taskbar_update_time(void);
                                void taskbar_update_status(void);
                                void taskbar_handle_click(int x, int y);

                                #endif



#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>
#include "kernel/timer.h"

#define TASKBAR_HEIGHT 28
#define SYSTEM_TRAY_WIDTH 200

typedef struct {
    char time_str[16];
        char date_str[32];
            uint8_t battery_level;
                uint8_t wifi_strength;
                    uint8_t volume_level;
                    } system_status_t;

                    typedef struct {
                        int x, y, width, height;
                            system_status_t status;
                                uint8_t visible;
                                } taskbar_t;

                                // Taskbar functions
                                void taskbar_init(void);
                                void taskbar_render(void);
                                void taskbar_update_time(void);
                                void taskbar_update_status(void);
                                void taskbar_handle_click(int x, int y);

                                #endif
