/**
 * Maya OS Programmable Interval Timer Driver
 * Updated: 2025-08-29 11:09:16 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/pit.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"

#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND 0x43

#define PIT_BASE_FREQUENCY 1193182
#define PIT_MODE_SQUARE_WAVE 0x36

static struct {
    uint32_t tick_count;
    uint32_t frequency;
    pit_callback_t callback;
    bool initialized;
} pit_state;

static void pit_handler(struct regs* r) {
    pit_state.tick_count++;

    if (pit_state.callback) {
        pit_state.callback(pit_state.tick_count);
    }
}

bool pit_init(uint32_t frequency) {
    if (pit_state.initialized) {
        return true;
    }

    if (frequency == 0 || frequency > PIT_BASE_FREQUENCY) {
        return false;
    }

    // Calculate divisor
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    if (divisor > 65535) {
        divisor = 65535;
    }

    // Set command byte
    outb(PIT_COMMAND, PIT_MODE_SQUARE_WAVE);

    // Set frequency divisor
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    // Install interrupt handler
    irq_install_handler(0, pit_handler);

    pit_state.tick_count = 0;
    pit_state.frequency = PIT_BASE_FREQUENCY / divisor;
    pit_state.callback = NULL;
    pit_state.initialized = true;

    return true;
}

void pit_set_callback(pit_callback_t callback) {
    if (!pit_state.initialized) {
        return;
    }
    pit_state.callback = callback;
}

uint32_t pit_get_tick_count(void) {
    return pit_state.tick_count;
}

uint32_t pit_get_frequency(void) {
    return pit_state.frequency;
}

void pit_sleep(uint32_t milliseconds) {
    if (!pit_state.initialized || milliseconds == 0) {
        return;
    }

    uint32_t target_ticks = pit_state.tick_count + 
                           (milliseconds * pit_state.frequency) / 1000;

    while (pit_state.tick_count < target_ticks) {
        __asm__ volatile("hlt");
    }
}

bool pit_is_initialized(void) {
    return pit_state.initialized;
}

void pit_cleanup(void) {
    if (!pit_state.initialized) {
        return;
    }

    // Disable PIT interrupt
    irq_install_handler(0, NULL);

    pit_state.initialized = false;
}
