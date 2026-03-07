/**
 * Maya OS Power Management
 * Uses ACPI/APM ports for power state transitions.
 * Author: AmanNagtodeOfficial
 */

#include "kernel/power.h"
#include "kernel/logging.h"
#include "libc/stdio.h"
#include "libc/string.h"

/* ACPI SCI / PM1a control port (default for QEMU / Bochs) */
#define ACPI_PM1A_CNT_PORT  0x0604  /* QEMU specific ACPI shutdown port */
#define ACPI_SHUTDOWN_VAL   0x2000  /* SLP_EN | SLP_TYP S5               */

/* APM fallback */
#define APM_PORT_CTRL       0xB002
#define APM_SHUTDOWN_VAL    0x0

static power_state_t current_state = POWER_STATE_S0;
static bool          power_initialized = false;

/* ─── helpers ──────────────────────────────────────────────────── */

static void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* ─── public API ───────────────────────────────────────────────── */

bool power_init(void) {
    if (power_initialized) return true;

    current_state    = POWER_STATE_S0;
    power_initialized = true;
    KLOG_I("Power management subsystem initialized (state=S0)");
    return true;
}

void power_shutdown(void) {
    KLOG_I("System shutdown requested");
    printf("\nMaya OS: Shutting down...\n");
    current_state = POWER_STATE_S5;

    /* ACPI S5 via QEMU/Bochs shutdown port */
    outw(ACPI_PM1A_CNT_PORT, ACPI_SHUTDOWN_VAL);

    /* APM fallback */
    outw(APM_PORT_CTRL, APM_SHUTDOWN_VAL);

    /* Software triple-fault as last resort */
    __asm__ volatile("cli; hlt");
    for (;;) __asm__ volatile("hlt");
}

void power_reboot(void) {
    KLOG_I("System reboot requested");
    printf("\nMaya OS: Rebooting...\n");
    current_state = POWER_STATE_S0;

    /* Pulse keyboard controller reset line */
    uint8_t good = 0x02;
    while (good & 0x02) {
        __asm__ volatile("inb $0x64, %0" : "=a"(good));
    }
    __asm__ volatile("outb %0, $0x64" : : "a"((uint8_t)0xFE));

    /* ACPI RESET via FADT reset register (assume port 0xCF9 as fallback) */
    __asm__ volatile("outb %0, $0xCF9" : : "a"((uint8_t)0x06));
    for (;;) __asm__ volatile("hlt");
}

void power_sleep(void) {
    KLOG_I("System sleep (S3) requested");
    printf("\nMaya OS: Entering sleep mode...\n");
    current_state = POWER_STATE_S3;
    /* Real S3: write SLP_TYP|SLP_EN to PM1a_CNT.
       Stub – on QEMU this is a no-op. */
    __asm__ volatile("hlt");
}

void power_hibernate(void) {
    KLOG_I("System hibernate (S4) requested");
    printf("\nMaya OS: Entering hibernate mode...\n");
    current_state = POWER_STATE_S4;
    /* Real S4: save to disk image, then enter S4 via ACPI.
       Stub – not implemented at hardware level. */
    __asm__ volatile("hlt");
}

power_state_t power_get_state(void) {
    return current_state;
}
