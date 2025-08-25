#include "drivers/ac97.h"
#include "drivers/pci.h"

#define AC97_VENDOR_ID_INTEL 0x8086
#define AC97_DEVICE_ID_ICH   0x2415

// AC97 mixer registers
#define AC97_MASTER_VOLUME    0x02
#define AC97_PCM_VOLUME       0x18
#define AC97_MIC_VOLUME       0x0E

typedef struct {
    uint32_t nabm_base;  // Native Audio Bus Master
    uint32_t nam_base;   // Native Audio Mixer
} ac97_controller_t;

void ac97_init(void);
void ac97_play_sound(uint8_t *buffer, uint32_t length);
void ac97_set_volume(uint8_t left, uint8_t right);