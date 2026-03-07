/**
 * Maya OS Programmable Interval Timer Driver
 * Updated: 2026-03-07 20:45:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/pit.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
#include "kernel/logging.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_BASE_FREQUENCY 1193182
#define PIT_MODE_SQUARE_WAVE 0x36

static struct {
    uint32_t ticks;
    uint32_t frequency;
    bool initialized;
} pit_state;

static void pit_handler(struct registers* r) {
    (void)r;
    pit_state.ticks++;
}

bool pit_init(void) {
    if (pit_state.initialized) return true;

    pit_state.frequency = 100; // Default 100Hz
    uint32_t divisor = PIT_BASE_FREQUENCY / pit_state.frequency;

    outb(PIT_COMMAND, PIT_MODE_SQUARE_WAVE);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    register_interrupt_handler(32, (isr_t)pit_handler); // IRQ0 is at 0x20 (32)

    pit_state.ticks = 0;
    pit_state.initialized = true;
    KLOG_I("PIT initialized at %u Hz.", pit_state.frequency);
    return true;
}

void pit_set_frequency(uint32_t freq) {
    if (!pit_state.initialized || freq == 0) return;

    pit_state.frequency = freq;
    uint32_t divisor = PIT_BASE_FREQUENCY / freq;
    if (divisor > 65535) divisor = 65535;

    outb(PIT_COMMAND, PIT_MODE_SQUARE_WAVE);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t pit_get_ticks(void) {
    return pit_state.ticks;
}

