#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_datetime_t;

bool rtc_init(void);
void rtc_get_datetime(rtc_datetime_t *dt);

#endif
