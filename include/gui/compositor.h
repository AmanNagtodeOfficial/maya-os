#ifndef GUI_COMPOSITOR_H
#define GUI_COMPOSITOR_H

#include <stdint.h>
#include <stdbool.h>

void compositor_init(void);
void compositor_begin_frame(void);
void compositor_end_frame(void);
void compositor_update_region(int x, int y, int w, int h);

#endif
