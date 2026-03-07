#ifndef TASKVIEW_H
#define TASKVIEW_H

#include <stdint.h>

void taskview_open(uint8_t alt_tab);
void taskview_close(void);
uint8_t taskview_is_open(void);
void taskview_draw(int screen_w, int screen_h);
void taskview_handle_key(char ascii, uint8_t scancode);
void taskview_handle_mouse(int x, int y, uint8_t buttons, int sw, int sh);

#endif
