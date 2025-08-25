
#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

#define FAT32_SECTOR_SIZE 512
#define FAT32_SIGNATURE 0xAA55
#define FAT32_EOC_MARK 0x0FFFFFF8

// FAT32 Boot Sector structure
typedef struct {
    uint8_t jump_code[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_dir_entries;  // 0 for FAT32
    uint16_t total_sectors_16;  // 0 for FAT32
    uint8_t media_descriptor;
    uint16_t fat_size_16;       // 0 for FAT32
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT32 specific fields
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_serial;
    char volume_label[11];
    char fs_type[8];
    uint8_t boot_code[420];
    uint16_t signature;
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 Directory Entry structure
typedef struct {
    char name[8];
    char extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

// FAT32 Long File Name entry
typedef struct {
    uint8_t order;
    uint16_t name1[5];
    uint8_t attributes;     // Always 0x0F
    uint8_t type;          // Always 0
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster_low;  // Always 0
    uint16_t name3[2];
} __attribute__((packed)) fat32_lfn_entry_t;

// File attributes
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN 0x02
#define FAT32_ATTR_SYSTEM 0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20
#define FAT32_ATTR_LFN 0x0F

// File system operations
void fat32_init(uint8_t drive);
int fat32_mount(uint8_t drive);
void fat32_unmount(void);

// File operations
int fat32_open(const char *filename, int mode);
int fat32_close(int fd);
int fat32_read(int fd, void *buffer, size_t size);
int fat32_write(int fd, const void *buffer, size_t size);
int fat32_seek(int fd, long offset, int whence);
long fat32_tell(int fd);
int fat32_eof(int fd);

// File management
int fat32_create_file(const char *filename);
int fat32_delete_file(const char *filename);
int fat32_rename_file(const char *old_name, const char *new_name);
int fat32_file_exists(const char *filename);
long fat32_get_file_size(const char *filename);

// Directory operations
int fat32_create_directory(const char *dirname);
int fat32_delete_directory(const char *dirname);
int fat32_change_directory(const char *dirname);
char *fat32_get_current_directory(void);

// Directory listing
typedef struct {
    char name[256];
    uint32_t size;
    uint8_t attributes;
    uint16_t date;
    uint16_t time;
} fat32_dir_info_t;

int fat32_list_directory(const char *dirname, fat32_dir_info_t **entries, int *count);
void fat32_free_directory_list(fat32_dir_info_t *entries, int count);

// Low-level functions
int fat32_read_file(const char *filename, uint8_t *buffer, uint32_t size);
int fat32_write_file(const char *filename, uint8_t *buffer, uint32_t size);
uint32_t fat32_get_next_cluster(uint32_t cluster);
uint32_t fat32_allocate_cluster(void);
void fat32_free_cluster(uint32_t cluster);
uint32_t fat32_find_free_cluster(void);

// Utility functions
void fat32_format(uint8_t drive, const char *volume_label);
int fat32_check_filesystem(void);
void fat32_defragment(void);
uint32_t fat32_get_free_space(void);
uint32_t fat32_get_total_space(void);

// Error codes
#define FAT32_OK 0
#define FAT32_ERROR_NOT_FOUND -1
#define FAT32_ERROR_ACCESS_DENIED -2
#define FAT32_ERROR_DISK_FULL -3
#define FAT32_ERROR_INVALID_PATH -4
#define FAT32_ERROR_FILE_EXISTS -5
#define FAT32_ERROR_NOT_A_FILE -6
#define FAT32_ERROR_NOT_A_DIRECTORY -7
#define FAT32_ERROR_INVALID_HANDLE -8
#define FAT32_ERROR_IO_ERROR -9
#define FAT32_ERROR_NOT_MOUNTED -10

#endif
