/**
 * Maya OS Real Time Clock Driver
 * Updated: 2026-03-07 20:42:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/rtc.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
#include "kernel/logging.h"
#include "libc/string.h"

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04
#define RTC_DAY 0x07
#define RTC_MONTH 0x08
#define RTC_YEAR 0x09
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B
#define RTC_STATUS_C 0x0C

static struct {
    rtc_datetime_t current_time;
    bool initialized;
    bool update_in_progress;
} rtc_state;

static uint8_t rtc_read_register(uint8_t reg) {
    outb(RTC_INDEX_PORT, reg);
    return inb(RTC_DATA_PORT);
}

static void rtc_write_register(uint8_t reg, uint8_t value) {
    outb(RTC_INDEX_PORT, reg);
    outb(RTC_DATA_PORT, value);
}

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

static void rtc_handler(struct registers* r) {
    (void)r;
    // Read Status Register C to acknowledge interrupt
    rtc_read_register(RTC_STATUS_C);

    if (rtc_state.update_in_progress) return;
    rtc_state.update_in_progress = true;

    rtc_datetime_t dt;
    dt.second = bcd_to_binary(rtc_read_register(RTC_SECONDS));
    dt.minute = bcd_to_binary(rtc_read_register(RTC_MINUTES));
    dt.hour   = bcd_to_binary(rtc_read_register(RTC_HOURS));
    dt.day    = bcd_to_binary(rtc_read_register(RTC_DAY));
    dt.month  = bcd_to_binary(rtc_read_register(RTC_MONTH));
    dt.year   = bcd_to_binary(rtc_read_register(RTC_YEAR)) + 2000;

    memcpy(&rtc_state.current_time, &dt, sizeof(rtc_datetime_t));

    rtc_state.update_in_progress = false;
}

bool rtc_init(void) {
    if (rtc_state.initialized) return true;

    // Disable NMI and select Status Register B
    outb(RTC_INDEX_PORT, 0x8B);
    uint8_t status_b = inb(RTC_DATA_PORT);
    
    // Enable periodic interrupt (0x40) and 24-hour mode (0x02)
    rtc_write_register(RTC_STATUS_B, status_b | 0x42);

    // Read Status Register C to clear any pending interrupts
    rtc_read_register(RTC_STATUS_C);

    // Install interrupt handler on IRQ8
    register_interrupt_handler(40, (isr_t)rtc_handler); // IRQ8 is usually at 0x28 (40)

    memset(&rtc_state, 0, sizeof(rtc_state));
    rtc_state.initialized = true;
    KLOG_I("RTC initialized.");
    return true;
}

void rtc_get_datetime(rtc_datetime_t *dt) {
    if (!rtc_state.initialized || !dt) return;

    // Wait if update is in progress
    while (rtc_read_register(RTC_STATUS_A) & 0x80);

    dt->second = bcd_to_binary(rtc_read_register(RTC_SECONDS));
    dt->minute = bcd_to_binary(rtc_read_register(RTC_MINUTES));
    dt->hour   = bcd_to_binary(rtc_read_register(RTC_HOURS));
    dt->day    = bcd_to_binary(rtc_read_register(RTC_DAY));
    dt->month  = bcd_to_binary(rtc_read_register(RTC_MONTH));
    dt->year   = bcd_to_binary(rtc_read_register(RTC_YEAR)) + 2000;
}

