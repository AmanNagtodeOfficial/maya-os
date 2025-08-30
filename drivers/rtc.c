/**
 * Maya OS Real Time Clock Driver
 * Updated: 2025-08-29 11:08:17 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/rtc.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
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
    rtc_time_t current_time;
    rtc_callback_t update_callback;
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

static void rtc_handler(struct regs* r) {
    // Read Status Register C to acknowledge interrupt
    rtc_read_register(RTC_STATUS_C);

    // Check if update is in progress
    if (rtc_state.update_in_progress) {
        return;
    }

    rtc_state.update_in_progress = true;

    // Read current time
    rtc_time_t new_time;
    new_time.second = bcd_to_binary(rtc_read_register(RTC_SECONDS));
    new_time.minute = bcd_to_binary(rtc_read_register(RTC_MINUTES));
    new_time.hour = bcd_to_binary(rtc_read_register(RTC_HOURS));
    new_time.day = bcd_to_binary(rtc_read_register(RTC_DAY));
    new_time.month = bcd_to_binary(rtc_read_register(RTC_MONTH));
    new_time.year = bcd_to_binary(rtc_read_register(RTC_YEAR)) + 2000;

    // Update current time
    memcpy(&rtc_state.current_time, &new_time, sizeof(rtc_time_t));

    // Call update callback if registered
    if (rtc_state.update_callback) {
        rtc_state.update_callback(&rtc_state.current_time);
    }

    rtc_state.update_in_progress = false;
}

bool rtc_init(void) {
    if (rtc_state.initialized) {
        return true;
    }

    // Disable NMI
    outb(RTC_INDEX_PORT, 0x8A);

    // Read Status Register A
    uint8_t status_a = rtc_read_register(RTC_STATUS_A);
    
    // Set rate to 1024 Hz (6)
    rtc_write_register(RTC_STATUS_A, (status_a & 0xF0) | 0x06);

    // Read Status Register B
    uint8_t status_b = rtc_read_register(RTC_STATUS_B);

    // Enable periodic interrupt, 24-hour mode
    rtc_write_register(RTC_STATUS_B, status_b | 0x42);

    // Read Status Register C to clear any pending interrupts
    rtc_read_register(RTC_STATUS_C);

    // Install interrupt handler
    irq_install_handler(8, rtc_handler);

    // Initialize state
    memset(&rtc_state, 0, sizeof(rtc_state));
    rtc_state.initialized = true;

    return true;
}

void rtc_set_callback(rtc_callback_t callback) {
    if (!rtc_state.initialized) {
        return;
    }
    rtc_state.update_callback = callback;
}

bool rtc_get_time(rtc_time_t* time) {
    if (!rtc_state.initialized || !time) {
        return false;
    }

    // Wait if update is in progress
    while (rtc_read_register(RTC_STATUS_A) & 0x80);

    // Read current time
    time->second = bcd_to_binary(rtc_read_register(RTC_SECONDS));
    time->minute = bcd_to_binary(rtc_read_register(RTC_MINUTES));
    time->hour = bcd_to_binary(rtc_read_register(RTC_HOURS));
    time->day = bcd_to_binary(rtc_read_register(RTC_DAY));
    time->month = bcd_to_binary(rtc_read_register(RTC_MONTH));
    time->year = bcd_to_binary(rtc_read_register(RTC_YEAR)) + 2000;

    return true;
}

bool rtc_is_initialized(void) {
    return rtc_state.initialized;
}

void rtc_cleanup(void) {
    if (!rtc_state.initialized) {
        return;
    }

    // Disable RTC interrupt
    uint8_t status_b = rtc_read_register(RTC_STATUS_B);
    rtc_write_register(RTC_STATUS_B, status_b & ~0x40);

    // Uninstall interrupt handler
    irq_install_handler(8, NULL);

    rtc_state.initialized = false;
}
