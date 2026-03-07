#ifndef AC97_H
#define AC97_H

#include <stdint.h>
#include <stdbool.h>

bool ac97_init(void);
void ac97_play_sound(uint8_t *buffer, uint32_t length);
void ac97_set_volume(uint8_t left, uint8_t right);
void ac97_stop(void);

#endif
