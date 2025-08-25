#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t frequency);
uint32_t timer_get_tick(void);
void timer_wait(uint32_t ticks);
void sleep(uint32_t ms);

#endif
