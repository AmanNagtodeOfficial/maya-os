/**
 * Maya OS Advanced Programmable Interrupt Controller (APIC) Driver
 * Updated: 2025-08-29 11:15:07 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/apic.h"
#include "kernel/memory.h"
#include "kernel/acpi.h"
#include "libc/string.h"

#define APIC_BASE_MSR 0x1B
#define APIC_BASE_MSR_ENABLE 0x800
#define APIC_SPURIOUS_VECTOR 0xFF

#define APIC_REG_ID 0x20
#define APIC_REG_VERSION 0x30
#define APIC_REG_TPR 0x80
#define APIC_REG_APR 0x90
#define APIC_REG_PPR 0xA0
#define APIC_REG_EOI 0xB0
#define APIC_REG_SVR 0xF0
#define APIC_REG_ICR_LOW 0x300
#define APIC_REG_ICR_HIGH 0x310
#define APIC_REG_LVT_TIMER 0x320
#define APIC_REG_LVT_THERMAL 0x330
#define APIC_REG_LVT_PERF 0x340
#define APIC_REG_LVT_LINT0 0x350
#define APIC_REG_LVT_LINT1 0x360
#define APIC_REG_LVT_ERROR 0x370
#define APIC_REG_TIMER_INIT 0x380
#define APIC_REG_TIMER_COUNT 0x390
#define APIC_REG_TIMER_DIV 0x3E0

typedef struct {
    acpi_header_t header;
    uint32_t local_apic_addr;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_entry_header_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_lapic_entry_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_addr;
    uint32_t global_system_interrupt_base;
} __attribute__((packed)) madt_ioapic_entry_t;

static struct {
    void* apic_base;
    void* ioapic_base;
    uint32_t bsp_apic_id;
    bool initialized;
} apic_state;

static uint32_t apic_read(uint32_t reg) {
    return *(volatile uint32_t*)((uintptr_t)apic_state.apic_base + reg);
}

static void apic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)((uintptr_t)apic_state.apic_base + reg) = value;
}

static uint32_t ioapic_read(uint32_t reg) {
    *(volatile uint32_t*)apic_state.ioapic_base = reg;
    return *(volatile uint32_t*)((uintptr_t)apic_state.ioapic_base + 0x10);
}

static void ioapic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)apic_state.ioapic_base = reg;
    *(volatile uint32_t*)((uintptr_t)apic_state.ioapic_base + 0x10) = value;
}

bool apic_init(void) {
    if (apic_state.initialized) {
        return true;
    }

    // Get MADT from ACPI
    acpi_madt_t* madt = acpi_get_table("APIC");
    if (!madt) {
        return false;
    }

    // Map APIC base address
    apic_state.apic_base = memory_map_physical(madt->local_apic_addr, 4096);
    if (!apic_state.apic_base) {
        return false;
    }

    // Parse MADT entries
    uint8_t* entry = (uint8_t*)(madt + 1);
    uint8_t* end = (uint8_t*)madt + madt->header.length;

    while (entry < end) {
        madt_entry_header_t* header = (madt_entry_header_t*)entry;

        switch (header->type) {
            case 0: { // Local APIC
                madt_lapic_entry_t* lapic = (madt_lapic_entry_t*)entry;
                if (lapic->flags & 1) { // Processor enabled
                    if (lapic->acpi_processor_id == 0) {
                        apic_state.bsp_apic_id = lapic->apic_id;
                    }
                }
                break;
            }

            case 1: { // I/O APIC
                madt_ioapic_entry_t* ioapic = (madt_ioapic_entry_t*)entry;
                apic_state.ioapic_base = memory_map_physical(ioapic->io_apic_addr, 4096);
                break;
            }
        }

        entry += header->length;
    }

    // Enable APIC
    uint64_t msr = rdmsr(APIC_BASE_MSR);
    msr |= APIC_BASE_MSR_ENABLE;
    wrmsr(APIC_BASE_MSR, msr);

    // Configure Spurious Interrupt Vector Register
    apic_write(APIC_REG_SVR, APIC_SPURIOUS_VECTOR | 0x100);

    // Disable all LVT entries
    apic_write(APIC_REG_LVT_TIMER, 0x10000);
    apic_write(APIC_REG_LVT_THERMAL, 0x10000);
    apic_write(APIC_REG_LVT_PERF, 0x10000);
    apic_write(APIC_REG_LVT_LINT0, 0x10000);
    apic_write(APIC_REG_LVT_LINT1, 0x10000);
    apic_write(APIC_REG_LVT_ERROR, 0x10000);

    apic_state.initialized = true;
    return true;
}

void apic_eoi(void) {
    if (!apic_state.initialized) {
        return;
    }
    apic_write(APIC_REG_EOI, 0);
}

void apic_send_ipi(uint32_t apic_id, uint32_t vector) {
    if (!apic_state.initialized) {
        return;
    }

    apic_write(APIC_REG_ICR_HIGH, apic_id << 24);
    apic_write(APIC_REG_ICR_LOW, vector);
}

uint32_t apic_get_id(void) {
    if (!apic_state.initialized) {
        return 0;
    }
    return (apic_read(APIC_REG_ID) >> 24) & 0xFF;
}

bool apic_is_bsp(void) {
    if (!apic_state.initialized) {
        return false;
    }
    return apic_get_id() == apic_state.bsp_apic_id;
}

void apic_set_timer(uint32_t vector, uint32_t initial_count, bool periodic) {
    if (!apic_state.initialized) {
        return;
    }

    // Configure timer divider (divide by 16)
    apic_write(APIC_REG_TIMER_DIV, 0x3);

    // Configure LVT Timer entry
    apic_write(APIC_REG_LVT_TIMER, vector | (periodic ? 0x20000 : 0));

    // Set initial count
    apic_write(APIC_REG_TIMER_INIT, initial_count);
}

uint32_t apic_get_timer_count(void) {
    if (!apic_state.initialized) {
        return 0;
    }
    return apic_read(APIC_REG_TIMER_COUNT);
}

void apic_stop_timer(void) {
    if (!apic_state.initialized) {
        return;
    }
    apic_write(APIC_REG_LVT_TIMER, 0x10000);
    apic_write(APIC_REG_TIMER_INIT, 0);
}

bool apic_is_initialized(void) {
    return apic_state.initialized;
}
