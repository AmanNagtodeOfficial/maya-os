#ifndef SETTINGS_APP_H
#define SETTINGS_APP_H

#include <stdint.h>

void settings_init(void);
void settings_draw(void);
void settings_handle_mouse(int x, int y, uint8_t buttons);
void settings_handle_key(char ascii);

#endif
