#include "fs/ext2.h"

#define EXT2_SIGNATURE 0xEF53
#define EXT2_BLOCK_SIZE 1024

typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t reserved_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    // ... more fields
} __attribute__((packed)) ext2_superblock_t;

int ext2_init(uint8_t drive);
int ext2_read_file(const char *path, uint8_t *buffer, uint32_t size);
int ext2_write_file(const char *path, uint8_t *data, uint32_t size);