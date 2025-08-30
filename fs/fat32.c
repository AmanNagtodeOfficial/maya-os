/**
 * Maya OS FAT32 Filesystem Driver
 * Updated: 2025-08-29 11:11:48 UTC
 * Author: AmanNagtodeOfficial
 */

#include "fs/fat32.h"
#include "drivers/ata.h"
#include "kernel/memory.h"
#include "libc/string.h"

#define FAT32_SIGNATURE 0xAA55
#define FAT32_EOC 0x0FFFFFF8
#define FAT32_CLUSTER_BAD 0x0FFFFFF7
#define FAT32_CLUSTER_FREE 0x00000000

typedef struct {
    uint8_t jump_boot[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t nt_reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

static struct {
    fat32_boot_sector_t* boot_sector;
    uint32_t* fat_table;
    uint32_t data_start;
    uint32_t clusters_count;
    uint8_t drive;
    bool mounted;
} fat32_fs;

static void* cluster_buffer = NULL;

bool fat32_init(uint8_t drive) {
    if (fat32_fs.mounted) {
        return true;
    }

    // Allocate buffers
    cluster_buffer = kmalloc(512);  // Start with sector size
    if (!cluster_buffer) {
        return false;
    }

    // Read boot sector
    if (!ata_read_sectors(drive, 0, 1, cluster_buffer)) {
        kfree(cluster_buffer);
        return false;
    }

    fat32_fs.boot_sector = kmalloc(sizeof(fat32_boot_sector_t));
    if (!fat32_fs.boot_sector) {
        kfree(cluster_buffer);
        return false;
    }
    memcpy(fat32_fs.boot_sector, cluster_buffer, sizeof(fat32_boot_sector_t));

    // Verify FAT32 signature
    uint16_t* signature = (uint16_t*)(cluster_buffer + 510);
    if (*signature != FAT32_SIGNATURE) {
        kfree(fat32_fs.boot_sector);
        kfree(cluster_buffer);
        return false;
    }

    // Calculate filesystem parameters
    uint32_t fat_size = fat32_fs.boot_sector->sectors_per_fat_32;
    uint32_t total_sectors = fat32_fs.boot_sector->total_sectors_32;
    fat32_fs.data_start = fat32_fs.boot_sector->reserved_sectors + 
                         (fat32_fs.boot_sector->num_fats * fat_size);
    fat32_fs.clusters_count = (total_sectors - fat32_fs.data_start) / 
                             fat32_fs.boot_sector->sectors_per_cluster;

    // Read FAT table
    uint32_t fat_sectors = fat32_fs.boot_sector->sectors_per_fat_32;
    fat32_fs.fat_table = kmalloc(fat_sectors * 512);
    if (!fat32_fs.fat_table) {
        kfree(fat32_fs.boot_sector);
        kfree(cluster_buffer);
        return false;
    }

    if (!ata_read_sectors(drive, fat32_fs.boot_sector->reserved_sectors,
                         fat_sectors, fat32_fs.fat_table)) {
        kfree(fat32_fs.fat_table);
        kfree(fat32_fs.boot_sector);
        kfree(cluster_buffer);
        return false;
    }

    fat32_fs.drive = drive;
    fat32_fs.mounted = true;
    return true;
}

static uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    return fat32_fs.data_start + 
           ((cluster - 2) * fat32_fs.boot_sector->sectors_per_cluster);
}

static bool fat32_read_cluster(uint32_t cluster, void* buffer) {
    if (!fat32_fs.mounted || cluster < 2 || 
        cluster >= fat32_fs.clusters_count + 2) {
        return false;
    }

    uint32_t lba = fat32_cluster_to_lba(cluster);
    return ata_read_sectors(fat32_fs.drive, lba,
                          fat32_fs.boot_sector->sectors_per_cluster, buffer);
}

bool fat32_read_file(const char* filename, void* buffer, size_t* size) {
    if (!fat32_fs.mounted || !filename || !buffer || !size) {
        return false;
    }

    // Convert filename to FAT format (8.3)
    char fat_name[11];
    memset(fat_name, ' ', 11);
    
    const char* dot = strchr(filename, '.');
    size_t name_len = dot ? (dot - filename) : strlen(filename);
    if (name_len > 8) name_len = 8;
    
    memcpy(fat_name, filename, name_len);
    
    if (dot && strlen(dot + 1) <= 3) {
        memcpy(fat_name + 8, dot + 1, strlen(dot + 1));
    }

    // Search root directory
    uint32_t current_cluster = fat32_fs.boot_sector->root_cluster;
    fat32_dir_entry_t* dir_entry;
    bool found = false;

    while (current_cluster >= 2 && current_cluster < FAT32_EOC) {
        if (!fat32_read_cluster(current_cluster, cluster_buffer)) {
            return false;
        }

        dir_entry = (fat32_dir_entry_t*)cluster_buffer;
        for (uint32_t i = 0; i < (512 * fat32_fs.boot_sector->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (memcmp(dir_entry[i].name, fat_name, 11) == 0) {
                found = true;
                
                // Get file size and cluster
                *size = dir_entry[i].file_size;
                current_cluster = (dir_entry[i].first_cluster_high << 16) |
                                 dir_entry[i].first_cluster_low;
                break;
            }
        }

        if (found) break;
        current_cluster = fat32_fs.fat_table[current_cluster];
    }

    if (!found) {
        return false;
    }

    // Read file data
    uint8_t* buf_ptr = (uint8_t*)buffer;
    size_t remaining = *size;

    while (current_cluster >= 2 && current_cluster < FAT32_EOC && remaining > 0) {
        if (!fat32_read_cluster(current_cluster, cluster_buffer)) {
            return false;
        }

        size_t to_copy = remaining;
        if (to_copy > 512 * fat32_fs.boot_sector->sectors_per_cluster) {
            to_copy = 512 * fat32_fs.boot_sector->sectors_per_cluster;
        }

        memcpy(buf_ptr, cluster_buffer, to_copy);
        buf_ptr += to_copy;
        remaining -= to_copy;

        current_cluster = fat32_fs.fat_table[current_cluster];
    }

    return true;
}

void fat32_cleanup(void) {
    if (!fat32_fs.mounted) {
        return;
    }

    if (fat32_fs.boot_sector) {
        kfree(fat32_fs.boot_sector);
    }
    if (fat32_fs.fat_table) {
        kfree(fat32_fs.fat_table);
    }
    if (cluster_buffer) {
        kfree(cluster_buffer);
    }

    memset(&fat32_fs, 0, sizeof(fat32_fs));
}

bool fat32_is_mounted(void) {
    return fat32_fs.mounted;
}
