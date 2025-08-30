/**
 * Maya OS System Timer Driver
 * Updated: 2025-08-29 10:55:59 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/timer.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"

#define PIT_FREQUENCY 1193180
#define PIT_DATA_PORT 0x40
#define PIT_COMMAND_PORT 0x43

static volatile uint32_t tick_count = 0;
static uint32_t ticks_per_second = 0;
static timer_callback_t timer_callbacks[MAX_TIMER_CALLBACKS];
static uint32_t callback_intervals[MAX_TIMER_CALLBACKS];
static uint32_t callback_counters[MAX_TIMER_CALLBACKS];
static bool timer_initialized = false;

static void timer_handler(struct regs* r) {
    tick_count++;

    // Process callbacks
    for (int i = 0; i < MAX_TIMER_CALLBACKS; i++) {
        if (timer_callbacks[i]) {
            callback_counters[i]++;
            if (callback_counters[i] >= callback_intervals[i]) {
                timer_callbacks[i]();
                callback_counters[i] = 0;
            }
        }
    }
}

bool timer_init(uint32_t frequency) {
    if (timer_initialized || frequency == 0) {
        return false;
    }

    // Calculate divisor
    uint32_t divisor = PIT_FREQUENCY / frequency;
    if (divisor > 65535) {
        divisor = 65535;
    }

    // Set PIT mode (mode 3, binary)
    outb(PIT_COMMAND_PORT, 0x36);

    // Set frequency divisor
    outb(PIT_DATA_PORT, divisor & 0xFF);
    outb(PIT_DATA_PORT, (divisor >> 8) & 0xFF);

    // Install timer handler
    irq_install_handler(0, timer_handler);

    ticks_per_second = frequency;
    timer_initialized = true;

    // Initialize callback arrays
    memset(timer_callbacks, 0, sizeof(timer_callbacks));
    memset(callback_intervals, 0, sizeof(callback_intervals));
    memset(callback_counters, 0, sizeof(callback_counters));

    return true;
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

void timer_wait(uint32_t ticks) {
    uint32_t start = tick_count;
    while (tick_count - start < ticks) {
        __asm__ volatile ("hlt");
    }
}

void timer_sleep(uint32_t milliseconds) {
    if (!timer_initialized || ticks_per_second == 0) {
        return;
    }

    uint32_t ticks = (milliseconds * ticks_per_second) / 1000;
    timer_wait(ticks);
}

int timer_add_callback(timer_callback_t callback, uint32_t interval_ms) {
    if (!timer_initialized || !callback || interval_ms == 0) {
        return -1;
    }

    // Find free slot
    for (int i = 0; i < MAX_TIMER_CALLBACKS; i++) {
        if (!timer_callbacks[i]) {
            timer_callbacks[i] = callback;
            callback_intervals[i] = (interval_ms * ticks_per_second) / 1000;
            callback_counters[i] = 0;
            return i;
        }
    }

    return -1;
}

bool timer_remove_callback(int callback_id) {
    if (!timer_initialized || callback_id < 0 || callback_id >= MAX_TIMER_CALLBACKS) {
        return false;
    }

    timer_callbacks[callback_id] = NULL;
    callback_intervals[callback_id] = 0;
    callback_counters[callback_id] = 0;
    return true;
}

bool timer_is_initialized(void) {
    return timer_initialized;
}

uint32_t timer_get_frequency(void) {
    return ticks_per_second;
}
