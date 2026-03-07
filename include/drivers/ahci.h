#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  fis_type;
    uint8_t  pmport:4;
    uint8_t  reserved0:3;
    uint8_t  c:1;
    uint8_t  command;
    uint8_t  featurel;
    uint8_t  lba0;
    uint8_t  lba1;
    uint8_t  lba2;
    uint8_t  device;
    uint8_t  lba3;
    uint8_t  lba4;
    uint8_t  lba5;
    uint8_t  featureh;
    uint8_t  countl;
    uint8_t  counth;
    uint8_t  icc;
    uint8_t  control;
    uint8_t  reserved1[4];
} fis_reg_h2d_t;

#define FIS_TYPE_REG_H2D 0x27
#define ATA_CMD_READ_DMA_EX 0x25

bool ahci_init(void);
bool ahci_read_sectors(uint32_t port, uint64_t start, uint32_t count, void* buffer);
bool ahci_write_sectors(uint32_t port, uint64_t start, uint32_t count, const void* buffer);

#endif
