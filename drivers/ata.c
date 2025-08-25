#include "drivers/ata.h"
#include "kernel/kernel.h"
#include "kernel/interrupts.h"
#include "libc/stdio.h"
#include "libc/string.h"

// ATA commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_FLUSH_CACHE 0xE7

// ATA status bits
#define ATA_STATUS_ERR  0x01
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_SRV  0x10
#define ATA_STATUS_DF   0x20
#define ATA_STATUS_RDY  0x40
#define ATA_STATUS_BSY  0x80

// Drive selection bits
#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE  0xB0

static uint8_t ata_drives[4] = {0}; // Track which drives exist

static void ata_wait_400ns(uint16_t port) {
    // Read alternate status register 4 times (400ns delay)
    inb(port + 6);
    inb(port + 6);
    inb(port + 6);
    inb(port + 6);
}

static uint8_t ata_wait_ready(uint16_t port) {
    uint32_t timeout = 10000;
    uint8_t status;
    
    while (timeout--) {
        status = inb(port + 7);
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_RDY)) {
            return 0; // Ready
        }
    }
    
    return 1; // Timeout
}

static uint8_t ata_wait_drq(uint16_t port) {
    uint32_t timeout = 10000;
    uint8_t status;
    
    while (timeout--) {
        status = inb(port + 7);
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            return 0; // Data ready
        }
        if (status & ATA_STATUS_ERR) {
            return 2; // Error
        }
    }
    
    return 1; // Timeout
}

static void ata_select_drive(uint8_t drive) {
    uint16_t port = (drive < 2) ? ATA_PRIMARY_DRIVE_HEAD : ATA_SECONDARY_DRIVE_HEAD;
    uint8_t drive_select = (drive & 1) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER;
    
    outb(port, drive_select);
    ata_wait_400ns((drive < 2) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA);
}

void ata_init(void) {
    printf("Initializing ATA/IDE drives...\n");
    
    // Check for primary master
    ata_select_drive(0);
    if (ata_wait_ready(ATA_PRIMARY_DATA) == 0) {
        ata_drives[0] = 1;
        printf("Primary master drive detected\n");
    }
    
    // Check for primary slave
    ata_select_drive(1);
    if (ata_wait_ready(ATA_PRIMARY_DATA) == 0) {
        ata_drives[1] = 1;
        printf("Primary slave drive detected\n");
    }
    
    // Check for secondary master
    ata_select_drive(2);
    if (ata_wait_ready(ATA_SECONDARY_DATA) == 0) {
        ata_drives[2] = 1;
        printf("Secondary master drive detected\n");
    }
    
    // Check for secondary slave
    ata_select_drive(3);
    if (ata_wait_ready(ATA_SECONDARY_DATA) == 0) {
        ata_drives[3] = 1;
        printf("Secondary slave drive detected\n");
    }
    
    printf("ATA initialization complete\n");
}

void ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, uint16_t *buffer) {
    if (drive >= 4 || !ata_drives[drive]) {
        printf("Invalid drive %d\n", drive);
        return;
    }
    
    uint16_t port_base = (drive < 2) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint8_t drive_select = ((drive & 1) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER) | 0x40 | ((lba >> 24) & 0x0F);
    
    // Select drive and set LBA mode
    ata_select_drive(drive);
    
    if (ata_wait_ready(port_base) != 0) {
        printf("Drive %d not ready\n", drive);
        return;
    }
    
    // Send read command
    outb(port_base + 6, drive_select);  // Drive/head register
    outb(port_base + 2, sectors);       // Sector count
    outb(port_base + 3, lba & 0xFF);    // LBA low
    outb(port_base + 4, (lba >> 8) & 0xFF);   // LBA mid
    outb(port_base + 5, (lba >> 16) & 0xFF);  // LBA high
    outb(port_base + 7, ATA_CMD_READ_SECTORS); // Command
    
    // Read sectors
    for (int i = 0; i < sectors; i++) {
        if (ata_wait_drq(port_base) != 0) {
            printf("Error reading sector %d\n", i);
            return;
        }
        
        // Read 256 words (512 bytes)
        for (int j = 0; j < 256; j++) {
            buffer[i * 256 + j] = inw(port_base);
        }
    }
}

void ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, uint16_t *buffer) {
    if (drive >= 4 || !ata_drives[drive]) {
        printf("Invalid drive %d\n", drive);
        return;
    }
    
    uint16_t port_base = (drive < 2) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint8_t drive_select = ((drive & 1) ? ATA_DRIVE_SLAVE : ATA_DRIVE_MASTER) | 0x40 | ((lba >> 24) & 0x0F);
    
    // Select drive and set LBA mode
    ata_select_drive(drive);
    
    if (ata_wait_ready(port_base) != 0) {
        printf("Drive %d not ready\n", drive);
        return;
    }
    
    // Send write command
    outb(port_base + 6, drive_select);  // Drive/head register
    outb(port_base + 2, sectors);       // Sector count
    outb(port_base + 3, lba & 0xFF);    // LBA low
    outb(port_base + 4, (lba >> 8) & 0xFF);   // LBA mid
    outb(port_base + 5, (lba >> 16) & 0xFF);  // LBA high
    outb(port_base + 7, ATA_CMD_WRITE_SECTORS); // Command
    
    // Write sectors
    for (int i = 0; i < sectors; i++) {
        if (ata_wait_drq(port_base) != 0) {
            printf("Error writing sector %d\n", i);
            return;
        }
        
        // Write 256 words (512 bytes)
        for (int j = 0; j < 256; j++) {
            outw(port_base, buffer[i * 256 + j]);
        }
    }
    
    // Flush cache
    outb(port_base + 7, ATA_CMD_FLUSH_CACHE);
    ata_wait_ready(port_base);
}

void ata_identify(uint8_t drive) {
    if (drive >= 4 || !ata_drives[drive]) {
        printf("Invalid drive %d\n", drive);
        return;
    }
    
    uint16_t port_base = (drive < 2) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint16_t identify_data[256];
    
    // Select drive
    ata_select_drive(drive);
    
    if (ata_wait_ready(port_base) != 0) {
        printf("Drive %d not ready\n", drive);
        return;
    }
    
    // Send identify command
    outb(port_base + 7, ATA_CMD_IDENTIFY);
    
    if (ata_wait_drq(port_base) != 0) {
        printf("Error identifying drive %d\n", drive);
        return;
    }
    
    // Read identify data
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(port_base);
    }
    
    // Parse identify data
    char model[41];
    char serial[21];
    
    // Extract model name (words 27-46)
    for (int i = 0; i < 20; i++) {
        uint16_t word = identify_data[27 + i];
        model[i * 2] = (word >> 8) & 0xFF;
        model[i * 2 + 1] = word & 0xFF;
    }
    model[40] = '\0';
    
    // Extract serial number (words 10-19)
    for (int i = 0; i < 10; i++) {
        uint16_t word = identify_data[10 + i];
        serial[i * 2] = (word >> 8) & 0xFF;
        serial[i * 2 + 1] = word & 0xFF;
    }
    serial[20] = '\0';
    
    // Get capacity
    uint32_t sectors = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    uint32_t capacity_mb = (sectors * 512) / (1024 * 1024);
    
    printf("Drive %d Information:\n", drive);
    printf("  Model: %s\n", model);
    printf("  Serial: %s\n", serial);
    printf("  Capacity: %d MB (%d sectors)\n", capacity_mb, sectors);
    printf("  Features: ");
    
    if (identify_data[49] & 0x0200) {
        printf("LBA ");
    }
    if (identify_data[83] & 0x0400) {
        printf("LBA48 ");
    }
    if (identify_data[49] & 0x0100) {
        printf("DMA ");
    }
    
    printf("\n");
}

// Helper function to check if drive exists
uint8_t ata_drive_exists(uint8_t drive) {
    return (drive < 4) ? ata_drives[drive] : 0;
}

// Get drive capacity in sectors
uint32_t ata_get_drive_size(uint8_t drive) {
    if (drive >= 4 || !ata_drives[drive]) {
        return 0;
    }
    
    uint16_t port_base = (drive < 2) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint16_t identify_data[256];
    
    ata_select_drive(drive);
    
    if (ata_wait_ready(port_base) != 0) {
        return 0;
    }
    
    outb(port_base + 7, ATA_CMD_IDENTIFY);
    
    if (ata_wait_drq(port_base) != 0) {
        return 0;
    }
    
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(port_base);
    }
    
    return ((uint32_t)identify_data[61] << 16) | identify_data[60];
}
