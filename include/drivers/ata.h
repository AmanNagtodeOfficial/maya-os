#ifndef ATA_H
#define ATA_H

#include <stdint.h>

#define ATA_PRIMARY_DATA 0x1F0
#define ATA_PRIMARY_ERR 0x1F1
#define ATA_PRIMARY_SECCOUNT 0x1F2
#define ATA_PRIMARY_LBA_LO 0x1F3
#define ATA_PRIMARY_LBA_MID 0x1F4
#define ATA_PRIMARY_LBA_HI 0x1F5
#define ATA_PRIMARY_DRIVE_HEAD 0x1F6
#define ATA_PRIMARY_COMM_REGSTAT 0x1F7
#define ATA_PRIMARY_ALTSTAT_DEVCTRL 0x3F6

#define ATA_SECONDARY_DATA 0x170
#define ATA_SECONDARY_ERR 0x171
#define ATA_SECONDARY_SECCOUNT 0x172
#define ATA_SECONDARY_LBA_LO 0x173
#define ATA_SECONDARY_LBA_MID 0x174
#define ATA_SECONDARY_LBA_HI 0x175
#define ATA_SECONDARY_DRIVE_HEAD 0x176
#define ATA_SECONDARY_COMM_REGSTAT 0x177
#define ATA_SECONDARY_ALTSTAT_DEVCTRL 0x376

#define SECTOR_SIZE 512

void ata_init(void);
void ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, uint16_t *buffer);
void ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t sectors, uint16_t *buffer);
void ata_identify(uint8_t drive);

#endif
