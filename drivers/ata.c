/**
 * Maya OS ATA Driver
 * Updated: 2025-08-29 11:15:07 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/ata.h"
#include "kernel/io.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERR 0x01

typedef struct {
    uint16_t flags;
    uint16_t cylinders;
    uint16_t reserved1;
    uint16_t heads;
    uint16_t reserved2[2];
    uint16_t sectors;
    uint16_t reserved3[3];
    char serial[20];
    uint16_t reserved4[2];
    uint16_t buffer_size;
    char firmware[8];
    char model[40];
    uint16_t reserved5[33];
    uint16_t capabilities;
    uint16_t reserved6[12];
    uint16_t sectors_28[2];
    uint16_t reserved7[22];
    uint16_t sectors_48[4];
} __attribute__((packed)) ata_identify_t;

static struct {
    uint16_t io_base;
    uint16_t control_base;
    bool is_slave;
    ata_identify_t identify;
    bool initialized;
} ata_state;

static void ata_wait_bsy(void) {
    while (inb(ata_state.io_base + ATA_REG_STATUS) & ATA_STATUS_BSY);
}

static void ata_wait_drq(void) {
    while (!(inb(ata_state.io_base + ATA_REG_STATUS) & ATA_STATUS_DRQ));
}

static bool ata_identify_drive(void) {
    // Select drive
    outb(ata_state.io_base + ATA_REG_HDDEVSEL, ata_state.is_slave ? 0xB0 : 0xA0);
    
    // Wait for drive to be ready
    ata_wait_bsy();

    // Send IDENTIFY command
    outb(ata_state.io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    // Check if drive exists
    uint8_t status = inb(ata_state.io_base + ATA_REG_STATUS);
    if (status == 0) {
        return false;
    }

    // Wait for data
    ata_wait_bsy();
    ata_wait_drq();

    // Read identify data
    for (int i = 0; i < 256; i++) {
        ((uint16_t*)&ata_state.identify)[i] = inw(ata_state.io_base + ATA_REG_DATA);
    }

    return true;
}

bool ata_init(bool is_slave) {
    if (ata_state.initialized) {
        return true;
    }

    ata_state.io_base = ATA_PRIMARY_IO;
    ata_state.control_base = ATA_PRIMARY_IO + 0x206;
    ata_state.is_slave = is_slave;

    // Software reset
    outb(ata_state.control_base, 0x04);
    outb(ata_state.control_base, 0x00);

    // Wait for drive to be ready
    ata_wait_bsy();

    // Identify drive
    if (!ata_identify_drive()) {
        return false;
    }

    ata_state.initialized = true;
    return true;
}

bool ata_read_sectors(uint32_t lba, uint8_t sector_count, void* buffer) {
    if (!ata_state.initialized || !buffer || sector_count == 0) {
        return false;
    }

    // Select drive and set LBA mode
    outb(ata_state.io_base + ATA_REG_HDDEVSEL, 
         (ata_state.is_slave ? 0xF0 : 0xE0) | ((lba >> 24) & 0x0F));

    // Set sector count and LBA registers
    outb(ata_state.io_base + ATA_REG_SECCOUNT0, sector_count);
    outb(ata_state.io_base + ATA_REG_LBA0, lba & 0xFF);
    outb(ata_state.io_base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(ata_state.io_base + ATA_REG_LBA2, (lba >> 16) & 0xFF);

    // Send read command
    outb(ata_state.io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    uint16_t* buf = (uint16_t*)buffer;
    for (uint8_t i = 0; i < sector_count; i++) {
        // Wait for data
        ata_wait_bsy();
        ata_wait_drq();

        // Read sector data
        for (int j = 0; j < 256; j++) {
            buf[j] = inw(ata_state.io_base + ATA_REG_DATA);
        }
        buf += 256;
    }

    return true;
}

bool ata_write_sectors(uint32_t lba, uint8_t sector_count, const void* buffer) {
    if (!ata_state.initialized || !buffer || sector_count == 0) {
        return false;
    }

    // Select drive and set LBA mode
    outb(ata_state.io_base + ATA_REG_HDDEVSEL, 
         (ata_state.is_slave ? 0xF0 : 0xE0) | ((lba >> 24) & 0x0F));

    // Set sector count and LBA registers
    outb(ata_state.io_base + ATA_REG_SECCOUNT0, sector_count);
    outb(ata_state.io_base + ATA_REG_LBA0, lba & 0xFF);
    outb(ata_state.io_base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(ata_state.io_base + ATA_REG_LBA2, (lba >> 16) & 0xFF);

    // Send write command
    outb(ata_state.io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    const uint16_t* buf = (const uint16_t*)buffer;
    for (uint8_t i = 0; i < sector_count; i++) {
        // Wait for drive to be ready
        ata_wait_bsy();
        ata_wait_drq();

        // Write sector data
        for (int j = 0; j < 256; j++) {
            outw(ata_state.io_base + ATA_REG_DATA, buf[j]);
        }
        buf += 256;
    }

    return true;
}

const char* ata_get_model(void) {
    if (!ata_state.initialized) {
        return NULL;
    }
    return ata_state.identify.model;
}

const char* ata_get_serial(void) {
    if (!ata_state.initialized) {
        return NULL;
    }
    return ata_state.identify.serial;
}

uint64_t ata_get_size(void) {
    if (!ata_state.initialized) {
        return 0;
    }

    if (ata_state.identify.capabilities & (1 << 9)) {
        // LBA48 supported
        return ((uint64_t)ata_state.identify.sectors_48[3] << 48) |
               ((uint64_t)ata_state.identify.sectors_48[2] << 32) |
               ((uint64_t)ata_state.identify.sectors_48[1] << 16) |
               ata_state.identify.sectors_48[0];
    } else {
        // LBA28
        return ((uint32_t)ata_state.identify.sectors_28[1] << 16) |
               ata_state.identify.sectors_28[0];
    }
}

bool ata_is_initialized(void) {
    return ata_state.initialized;
}
