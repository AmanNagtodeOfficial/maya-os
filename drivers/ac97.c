/**
 * Maya OS AC97 Audio Driver
 * Updated: 2026-03-07 20:30:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/ac97.h"
#include "drivers/pci.h"
#include "kernel/io.h"
#include "kernel/logging.h"
#include "libc/stdio.h"

#define AC97_VENDOR_ID_INTEL 0x8086
#define AC97_DEVICE_ID_ICH   0x2415

// AC97 mixer registers
#define AC97_MASTER_VOLUME    0x02
#define AC97_PCM_VOLUME       0x18
#define AC97_MIC_VOLUME       0x0E

// Audio Bus Master (NABM) registers
#define AC97_PCM_IN_CR        0x1B
#define AC97_PCM_OUT_CR       0x2B
#define AC97_MIC_IN_CR        0x3B

static struct {
    uint32_t nabm_base;
    uint32_t nam_base;
    bool initialized;
} ac97_state;

bool ac97_init(void) {
    if (ac97_state.initialized) {
        return true;
    }

    pci_device_t* dev = pci_find_device(AC97_VENDOR_ID_INTEL, AC97_DEVICE_ID_ICH);
    if (!dev) {
        KLOG_W("AC97 audio controller not found.");
        return false;
    }

    ac97_state.nam_base = dev->bars[0] & ~0xF;
    ac97_state.nabm_base = dev->bars[1] & ~0xF;

    // Enable PCI bus mastering
    pci_enable_bus_mastering(dev->bus, dev->slot, dev->function);

    // Warm reset AC97
    outw(ac97_state.nam_base + 0x00, 0x0001);

    // Set default volume
    ac97_set_volume(0x0, 0x0); // 0.0 is max volume for AC97 mixer (logarithmic)

    ac97_state.initialized = true;
    KLOG_I("AC97 audio controller initialized.");
    return true;
}

void ac97_play_sound(uint8_t *buffer, uint32_t length) {
    if (!ac97_state.initialized) return;
    
    // DMA setup would go here:
    // 1. Prepare Buffer Descriptor List (BDL)
    // 2. Point NABM_BASE + 0x10 to BDL physical address
    // 3. Set NABM_BASE + 0x15 to Last Valid Entry
    // 4. Set PCM_OUT_CR (0x2B) to start
    
    (void)buffer;
    (void)length;
    KLOG_D("AC97: Sound playback triggered (stub).");
}

void ac97_set_volume(uint8_t left, uint8_t right) {
    if (!ac97_state.initialized) return;
    
    // In AC97, 0 is max volume, 31 is min (mute bit is 0x8000)
    uint16_t volume = ((left & 0x1F) << 8) | (right & 0x1F);
    outw(ac97_state.nam_base + AC97_MASTER_VOLUME, volume);
}

void ac97_stop(void) {
    if (!ac97_state.initialized) return;
    outb(ac97_state.nabm_base + AC97_PCM_OUT_CR, 0x00);
}
