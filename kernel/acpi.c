/**
 * Maya OS ACPI Implementation
 * Updated: 2025-08-29 11:14:06 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/acpi.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define ACPI_RSDP_SIGNATURE "RSD PTR "
#define ACPI_RSDT_SIGNATURE "RSDT"
#define ACPI_XSDT_SIGNATURE "XSDT"
#define ACPI_FADT_SIGNATURE "FACP"
#define ACPI_MADT_SIGNATURE "APIC"

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed)) acpi_rsdp_t;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

typedef struct {
    acpi_header_t header;
    uint32_t tables[];
} __attribute__((packed)) acpi_rsdt_t;

typedef struct {
    acpi_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cst_control;
    uint16_t c2_latency;
    uint16_t c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t boot_flags;
    uint8_t reserved2;
    uint32_t flags;
} __attribute__((packed)) acpi_fadt_t;

static struct {
    acpi_rsdp_t* rsdp;
    acpi_rsdt_t* rsdt;
    acpi_fadt_t* fadt;
    bool initialized;
} acpi_state;

static bool acpi_validate_table(const void* table, size_t length) {
    const uint8_t* ptr = table;
    uint8_t sum = 0;

    for (size_t i = 0; i < length; i++) {
        sum += ptr[i];
    }

    return sum == 0;
}

static void* acpi_find_table(const char* signature) {
    uint32_t entries = (acpi_state.rsdt->header.length - sizeof(acpi_header_t)) / 4;

    for (uint32_t i = 0; i < entries; i++) {
        acpi_header_t* header = (acpi_header_t*)acpi_state.rsdt->tables[i];
        
        if (memcmp(header->signature, signature, 4) == 0) {
            if (acpi_validate_table(header, header->length)) {
                return header;
            }
        }
    }

    return NULL;
}

bool acpi_init(void) {
    if (acpi_state.initialized) {
        return true;
    }

    // Search for RSDP in BIOS memory areas
    for (uintptr_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        if (memcmp((void*)addr, ACPI_RSDP_SIGNATURE, 8) == 0) {
            acpi_state.rsdp = (acpi_rsdp_t*)addr;
            break;
        }
    }

    if (!acpi_state.rsdp) {
        return false;
    }

    // Validate RSDP
    if (!acpi_validate_table(acpi_state.rsdp, sizeof(acpi_rsdp_t))) {
        return false;
    }

    // Get RSDT
    acpi_state.rsdt = (acpi_rsdt_t*)acpi_state.rsdp->rsdt_address;
    if (!acpi_validate_table(acpi_state.rsdt, acpi_state.rsdt->header.length)) {
        return false;
    }

    // Find FADT
    acpi_state.fadt = acpi_find_table(ACPI_FADT_SIGNATURE);
    if (!acpi_state.fadt) {
        return false;
    }

    acpi_state.initialized = true;
    return true;
}

void acpi_enable(void) {
    if (!acpi_state.initialized) {
        return;
    }

    // Enable ACPI mode
    if (acpi_state.fadt->smi_command_port && acpi_state.fadt->acpi_enable) {
        outb(acpi_state.fadt->smi_command_port, acpi_state.fadt->acpi_enable);
        
        // Wait for ACPI to be enabled
        while ((inw(acpi_state.fadt->pm1a_control_block) & 1) == 0);
    }
}

void acpi_shutdown(void) {
    if (!acpi_state.initialized) {
        return;
    }

    // Send shutdown command
    outw(acpi_state.fadt->pm1a_control_block, 
         (1 << 13) | (1 << 12) | (1 << 5));
}

void* acpi_get_table(const char* signature) {
    if (!acpi_state.initialized) {
        return NULL;
    }

    return acpi_find_table(signature);
}

bool acpi_is_initialized(void) {
    return acpi_state.initialized;
}
