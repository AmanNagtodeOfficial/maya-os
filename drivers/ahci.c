/**
 * Maya OS AHCI (Advanced Host Controller Interface) Driver
 * Updated: 2025-08-29 11:21:54 UTC
 * Author: AmanNagtodeOfficial
 */

#include "drivers/ahci.h"
#include "drivers/pci.h"
#include "kernel/memory.h"
#include "kernel/interrupt.h"
#include "libc/string.h"

#define AHCI_VENDOR_ID 0x8086  // Intel
#define AHCI_BASE_CLASS 0x01   // Mass Storage
#define AHCI_SUB_CLASS 0x06    // SATA

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define HBA_PxIS_TFES   (1 << 30)

#define AHCI_MAX_PORTS 32
#define AHCI_MAX_COMMANDS 32
#define AHCI_SECTOR_SIZE 512

typedef volatile struct {
    uint32_t clb;           // Command List Base Address
    uint32_t clbu;          // Command List Base Address Upper 32-bits
    uint32_t fb;            // FIS Base Address
    uint32_t fbu;           // FIS Base Address Upper 32-bits
    uint32_t is;            // Interrupt Status
    uint32_t ie;            // Interrupt Enable
    uint32_t cmd;           // Command and Status
    uint32_t reserved0;     // Reserved
    uint32_t tfd;          // Task File Data
    uint32_t sig;          // Signature
    uint32_t ssts;         // SATA Status
    uint32_t sctl;         // SATA Control
    uint32_t serr;         // SATA Error
    uint32_t sact;         // SATA Active
    uint32_t ci;           // Command Issue
    uint32_t sntf;         // SATA Notification
    uint32_t fbs;          // FIS-based Switching Control
    uint32_t reserved1[11];// Reserved
    uint32_t vendor[4];    // Vendor Specific
} hba_port_t;

typedef volatile struct {
    uint32_t cap;          // Host Capabilities
    uint32_t ghc;          // Global Host Control
    uint32_t is;           // Interrupt Status
    uint32_t pi;           // Ports Implemented
    uint32_t vs;           // Version
    uint32_t ccc_ctl;      // Command Completion Coalescing Control
    uint32_t ccc_pts;      // Command Completion Coalescing Ports
    uint32_t em_loc;       // Enclosure Management Location
    uint32_t em_ctl;       // Enclosure Management Control
    uint32_t cap2;         // Host Capabilities Extended
    uint32_t bohc;         // BIOS/OS Handoff Control and Status
    uint8_t  reserved[0xA0-0x2C];
    uint8_t  vendor[0x100-0xA0];
    hba_port_t ports[AHCI_MAX_PORTS];
} hba_mem_t;

typedef struct {
    uint8_t  cfl:5;        // Command FIS Length
    uint8_t  a:1;          // ATAPI
    uint8_t  w:1;          // Write
    uint8_t  p:1;          // Prefetchable
    uint8_t  r:1;          // Reset
    uint8_t  b:1;          // BIST
    uint8_t  c:1;          // Clear Busy upon R_OK
    uint8_t  reserved0:1;
    uint8_t  pmp:4;        // Port Multiplier Port
    uint16_t prdtl;        // Physical Region Descriptor Table Length
    uint32_t prdbc;        // Physical Region Descriptor Byte Count
    uint32_t ctba;         // Command Table Base Address
    uint32_t ctbau;        // Command Table Base Address Upper 32-bits
    uint32_t reserved1[4];
} hba_cmd_header_t;

typedef struct {
    uint32_t dba;          // Data Base Address
    uint32_t dbau;         // Data Base Address Upper 32-bits
    uint32_t reserved0;
    uint32_t dbc:22;       // Byte Count
    uint32_t reserved1:9;
    uint32_t i:1;          // Interrupt on Completion
} hba_prdt_entry_t;

typedef struct {
    uint8_t  cfis[64];     // Command FIS
    uint8_t  acmd[16];     // ATAPI Command
    uint8_t  reserved[48];
    hba_prdt_entry_t prdt[]; // Physical Region Descriptor Table
} hba_cmd_tbl_t;

static struct {
    hba_mem_t* hba_mem;
    void* cmd_list[AHCI_MAX_PORTS];
    void* fis_base[AHCI_MAX_PORTS];
    void* cmd_tables[AHCI_MAX_PORTS][AHCI_MAX_COMMANDS];
    uint32_t port_count;
    bool initialized;
} ahci_state;

static int ahci_find_cmdslot(hba_port_t* port) {
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < AHCI_MAX_COMMANDS; i++) {
        if ((slots & (1 << i)) == 0) {
            return i;
        }
    }
    return -1;
}

static bool ahci_port_initialize(hba_port_t* port, uint32_t port_num) {
    // Stop command processing
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;

    // Wait until FR and CR are cleared
    while (port->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));

    // Allocate command list
    ahci_state.cmd_list[port_num] = memory_alloc_dma(1024, 1024);
    if (!ahci_state.cmd_list[port_num]) {
        return false;
    }
    memset(ahci_state.cmd_list[port_num], 0, 1024);

    // Set command list base address
    port->clb = (uint32_t)memory_get_physical(ahci_state.cmd_list[port_num]);
    port->clbu = 0;

    // Allocate FIS base
    ahci_state.fis_base[port_num] = memory_alloc_dma(256, 256);
    if (!ahci_state.fis_base[port_num]) {
        memory_free_dma(ahci_state.cmd_list[port_num]);
        return false;
    }
    memset(ahci_state.fis_base[port_num], 0, 256);

    // Set FIS base address
    port->fb = (uint32_t)memory_get_physical(ahci_state.fis_base[port_num]);
    port->fbu = 0;

    // Allocate command tables
    for (int i = 0; i < AHCI_MAX_COMMANDS; i++) {
        ahci_state.cmd_tables[port_num][i] = memory_alloc_dma(256, 256);
        if (!ahci_state.cmd_tables[port_num][i]) {
            // Clean up previously allocated resources
            for (int j = 0; j < i; j++) {
                memory_free_dma(ahci_state.cmd_tables[port_num][j]);
            }
            memory_free_dma(ahci_state.fis_base[port_num]);
            memory_free_dma(ahci_state.cmd_list[port_num]);
            return false;
        }
        memset(ahci_state.cmd_tables[port_num][i], 0, 256);
    }

    // Start command processing
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;

    return true;
}

bool ahci_init(void) {
    if (ahci_state.initialized) {
        return true;
    }

    // Find AHCI controller
    for (uint32_t i = 0; i < pci_get_device_count(); i++) {
        const pci_device_info_t* dev = pci_get_device(i);
        if (dev->class_code == AHCI_BASE_CLASS &&
            dev->subclass == AHCI_SUB_CLASS) {
            // Found AHCI controller
            uint32_t bar5 = pci_get_bar_address(dev, 5);
            if (!bar5) {
                continue;
            }

            // Enable bus mastering
            pci_enable_bus_mastering(dev->bus, dev->device, dev->function);

            // Map AHCI BAR
            ahci_state.hba_mem = memory_map_physical(bar5, sizeof(hba_mem_t));
            if (!ahci_state.hba_mem) {
                return false;
            }

            // Reset HBA
            ahci_state.hba_mem->ghc |= (1 << 0);
            while (ahci_state.hba_mem->ghc & (1 << 0));

            // Enable AHCI mode
            ahci_state.hba_mem->ghc |= (1 << 31);

            // Initialize ports
            uint32_t pi = ahci_state.hba_mem->pi;
            for (int i = 0; i < AHCI_MAX_PORTS; i++) {
                if (!(pi & (1 << i))) {
                    continue;
                }

                hba_port_t* port = &ahci_state.hba_mem->ports[i];
                uint8_t det = (port->ssts >> 0) & 0x0F;
                uint8_t ipm = (port->ssts >> 8) & 0x0F;

                if (det != HBA_PORT_DET_PRESENT || 
                    ipm != HBA_PORT_IPM_ACTIVE) {
                    continue;
                }

                if (!ahci_port_initialize(port, ahci_state.port_count)) {
                    // Clean up on failure
                    for (uint32_t j = 0; j < ahci_state.port_count; j++) {
                        for (int k = 0; k < AHCI_MAX_COMMANDS; k++) {
                            memory_free_dma(ahci_state.cmd_tables[j][k]);
                        }
                        memory_free_dma(ahci_state.fis_base[j]);
                        memory_free_dma(ahci_state.cmd_list[j]);
                    }
                    memory_unmap(ahci_state.hba_mem);
                    return false;
                }

                ahci_state.port_count++;
            }

            ahci_state.initialized = true;
            return true;
        }
    }

    return false;
}

bool ahci_read_sectors(uint32_t port, uint64_t start, uint32_t count, void* buffer) {
    if (!ahci_state.initialized || port >= ahci_state.port_count || 
        !buffer || count == 0) {
        return false;
    }

    hba_port_t* port_regs = &ahci_state.hba_mem->ports[port];

    // Find a free command slot
    int slot = ahci_find_cmdslot(port_regs);
    if (slot == -1) {
        return false;
    }

    hba_cmd_header_t* cmd_header = (hba_cmd_header_t*)ahci_state.cmd_list[port];
    cmd_header += slot;
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / 4;  // Command FIS size
    cmd_header->w = 0;  // Read from device
    cmd_header->prdtl = (uint16_t)((count - 1) >> 4) + 1;  // PRDT entries count

    hba_cmd_tbl_t* cmd_tbl = (hba_cmd_tbl_t*)ahci_state.cmd_tables[port][slot];
    memset(cmd_tbl, 0, sizeof(hba_cmd_tbl_t) + 
           (cmd_header->prdtl * sizeof(hba_prdt_entry_t)));

    // Setup PRDT
    int prd_count = (count * AHCI_SECTOR_SIZE + 0x1FFFF) >> 16;
    for (int i = 0; i < prd_count; i++) {
        cmd_tbl->prdt[i].dba = (uint32_t)memory_get_physical(buffer) + (i * 0x10000);
        cmd_tbl->prdt[i].dbau = 0;
        cmd_tbl->prdt[i].dbc = 0xFFFF;  // 64KB
        cmd_tbl->prdt[i].i = 1;
    }

    // Last entry
    cmd_tbl->prdt[prd_count-1].dbc = ((count * AHCI_SECTOR_SIZE - 1) & 0xFFFF) + 1;

    // Setup command
    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)(&cmd_tbl->cfis[0]);
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)start;
    cmdfis->lba1 = (uint8_t)(start >> 8);
    cmdfis->lba2 = (uint8_t)(start >> 16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (uint8_t)(start >> 24);
    cmdfis->lba4 = (uint8_t)(start >> 32);
    cmdfis->lba5 = (uint8_t)(start >> 40);

    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    // Issue command
    port_regs->ci = 1 << slot;

    // Wait for completion
    while ((port_regs->ci & (1 << slot)) != 0) {
        if (port_regs->is & HBA_PxIS_TFES) {
            return false;  // Task file error
        }
    }

    return !(port_regs->is & HBA_PxIS_TFES);
}

bool ahci_write_sectors(uint32_t port, uint64_t start, uint32_t count, const void* buffer) {
    // Similar to read_sectors but with write command
    // Implementation follows the same pattern with appropriate modifications
    return false; // TODO: Implement
}

uint32_t ahci_get_port_count(void) {
    return ahci_state.port_count;
}

bool ahci_is_initialized(void) {
    return ahci_state.initialized;
}
