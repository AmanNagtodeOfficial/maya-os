#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include <stdbool.h>

bool pit_init(void);
void pit_set_frequency(uint32_t freq);
uint32_t pit_get_ticks(void);

#endif
