/**
 * Maya OS System Timer Implementation
 * Updated: 2025-08-29 11:16:18 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/timer.h"
#include "kernel/apic.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define TIMER_FREQUENCY 1000 // 1000 Hz
#define TIMER_VECTOR 32

static struct {
    uint32_t ticks;
    timer_callback_t callback;
    bool initialized;
} timer_state;

static void timer_handler(struct regs* r) {
    timer_state.ticks++;

    if (timer_state.callback) {
        timer_state.callback(timer_state.ticks);
    }

    apic_eoi();
}

bool timer_init(void) {
    if (timer_state.initialized) {
        return true;
    }

    // Register interrupt handler
    interrupt_register_handler(TIMER_VECTOR, timer_handler);

    // Calculate APIC timer initial count for desired frequency
    uint32_t bus_frequency = 200000000; // 200MHz (example, should be detected)
    uint32_t initial_count = bus_frequency / (16 * TIMER_FREQUENCY);

    // Configure APIC timer
    apic_set_timer(TIMER_VECTOR, initial_count, true);

    timer_state.ticks = 0;
    timer_state.callback = NULL;
    timer_state.initialized = true;

    return true;
}

void timer_set_callback(timer_callback_t callback) {
    if (!timer_state.initialized) {
        return;
    }
    timer_state.callback = callback;
}

uint32_t timer_get_ticks(void) {
    return timer_state.ticks;
}

void timer_sleep(uint32_t milliseconds) {
    if (!timer_state.initialized) {
        return;
    }

    uint32_t target_ticks = timer_state.ticks + 
                           (milliseconds * TIMER_FREQUENCY) / 1000;

    while (timer_state.ticks < target_ticks) {
        __asm__ volatile("hlt");
    }
}

uint64_t timer_get_uptime(void) {
    if (!timer_state.initialized) {
        return 0;
    }

    return (uint64_t)timer_state.ticks * 1000 / TIMER_FREQUENCY;
}

void timer_calibrate(void) {
    if (!timer_state.initialized) {
        return;
    }

    // Stop timer
    apic_stop_timer();

    // Set maximum initial count
    apic_set_timer(TIMER_VECTOR, 0xFFFFFFFF, false);

    // Wait for 10ms using another time source (e.g., PIT)
    // TODO: Implement precise delay using PIT

    // Read remaining count
    uint32_t remaining = apic_get_timer_count();
    uint32_t ticks = 0xFFFFFFFF - remaining;

    // Calculate bus frequency
    uint32_t bus_frequency = ticks * 16 * 100; // ticks * divider * frequency

    // Reconfigure timer with calibrated frequency
    uint32_t initial_count = bus_frequency / (16 * TIMER_FREQUENCY);
    apic_set_timer(TIMER_VECTOR, initial_count, true);
}

bool timer_is_initialized(void) {
    return timer_state.initialized;
}
