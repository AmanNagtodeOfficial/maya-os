/**
 * Maya OS ext2 Filesystem Implementation
 * Updated: 2025-08-29 11:22:38 UTC
 * Author: AmanNagtodeOfficial
 */

#include "fs/ext2.h"
#include "kernel/memory.h"
#include "drivers/ata.h"
#include "libc/string.h"

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE 1024

#define EXT2_ROOT_INODE 2

#define EXT2_S_IFREG  0x8000
#define EXT2_S_IFDIR  0x4000

static struct {
    ext2_superblock_t superblock;
    ext2_bgd_t* block_group_descs;
    uint32_t block_size;
    uint32_t block_group_count;
    bool initialized;
} ext2_state;

static bool ext2_read_block(uint32_t block, void* buffer) {
    uint32_t sectors_per_block = ext2_state.block_size / ata_get_sector_size();
    return ata_read_sectors(block * sectors_per_block, sectors_per_block, buffer);
}

static bool ext2_write_block(uint32_t block, const void* buffer) {
    uint32_t sectors_per_block = ext2_state.block_size / ata_get_sector_size();
    return ata_write_sectors(block * sectors_per_block, sectors_per_block, buffer);
}

static bool ext2_read_inode(uint32_t inode_num, ext2_inode_t* inode) {
    if (inode_num == 0) {
        return false;
    }

    uint32_t block_group = (inode_num - 1) / ext2_state.superblock.s_inodes_per_group;
    uint32_t index = (inode_num - 1) % ext2_state.superblock.s_inodes_per_group;
    uint32_t block = ext2_state.block_group_descs[block_group].bg_inode_table;
    uint32_t offset = index * ext2_state.superblock.s_inode_size;

    uint8_t* buffer = kmalloc(ext2_state.block_size);
    if (!buffer) {
        return false;
    }

    if (!ext2_read_block(block + (offset / ext2_state.block_size), buffer)) {
        kfree(buffer);
        return false;
    }

    memcpy(inode, buffer + (offset % ext2_state.block_size), sizeof(ext2_inode_t));
    kfree(buffer);
    return true;
}

static uint32_t ext2_get_block_from_inode(ext2_inode_t* inode, uint32_t block_num) {
    uint32_t p_per_block = ext2_state.block_size / sizeof(uint32_t);

    if (block_num < EXT2_NDIR_BLOCKS) {
        // Direct blocks
        return inode->i_block[block_num];
    } else if (block_num < EXT2_NDIR_BLOCKS + p_per_block) {
        // Singly indirect blocks
        uint32_t* indirect_block = kmalloc(ext2_state.block_size);
        if (!indirect_block) return 0;
        ext2_read_block(inode->i_block[EXT2_IND_BLOCK], indirect_block);
        uint32_t block = indirect_block[block_num - EXT2_NDIR_BLOCKS];
        kfree(indirect_block);
        return block;
    } else if (block_num < EXT2_NDIR_BLOCKS + p_per_block + p_per_block * p_per_block) {
        // Doubly indirect blocks
        // ... implementation needed ...
    } else {
        // Triply indirect blocks
        // ... implementation needed ...
    }

    return 0; // Not implemented for deeper levels
}

bool ext2_init(void) {
    if (ext2_state.initialized) {
        return true;
    }

    // Read superblock
    if (ata_read_sectors(EXT2_SUPERBLOCK_OFFSET / ata_get_sector_size(),
                         sizeof(ext2_superblock_t) / ata_get_sector_size(),
                         &ext2_state.superblock) != 0) {
        return false;
    }

    // Check magic number
    if (ext2_state.superblock.s_magic != EXT2_SUPER_MAGIC) {
        return false;
    }

    ext2_state.block_size = 1024 << ext2_state.superblock.s_log_block_size;
    ext2_state.block_group_count = (ext2_state.superblock.s_blocks_count +
                                   ext2_state.superblock.s_blocks_per_group - 1) /
                                   ext2_state.superblock.s_blocks_per_group;

    // Read block group descriptors
    uint32_t bgd_size = ext2_state.block_group_count * sizeof(ext2_bgd_t);
    ext2_state.block_group_descs = kmalloc(bgd_size);
    if (!ext2_state.block_group_descs) {
        return false;
    }

    uint32_t bgd_block = (EXT2_SUPERBLOCK_OFFSET + EXT2_SUPERBLOCK_SIZE) / ext2_state.block_size;
    if (!ext2_read_block(bgd_block, ext2_state.block_group_descs)) {
        kfree(ext2_state.block_group_descs);
        return false;
    }

    ext2_state.initialized = true;
    return true;
}

fs_node_t* ext2_mount(void) {
    if (!ext2_state.initialized) {
        if (!ext2_init()) {
            return NULL;
        }
    }

    ext2_inode_t root_inode_data;
    if (!ext2_read_inode(EXT2_ROOT_INODE, &root_inode_data)) {
        return NULL;
    }

    fs_node_t* root_node = kmalloc(sizeof(fs_node_t));
    if (!root_node) {
        return NULL;
    }

    strcpy(root_node->name, "/");
    root_node->inode = EXT2_ROOT_INODE;
    root_node->flags = FS_DIRECTORY;
    // ... other fs_node fields ...

    return root_node;
}

ssize_t ext2_read(fs_node_t* node, uint64_t offset, size_t size, void* buffer) {
    if (!node || !buffer || !(node->flags & FS_FILE)) {
        return -1;
    }

    ext2_inode_t inode_data;
    if (!ext2_read_inode(node->inode, &inode_data)) {
        return -1;
    }

    if (offset >= inode_data.i_.size) {
        return 0;
    }
    if (offset + size > inode_data.i_size) {
        size = inode_data.i_size - offset;
    }

    uint32_t start_block = offset / ext2_state.block_size;
    uint32_t end_block = (offset + size - 1) / ext2_state.block_size;
    uint32_t offset_in_block = offset % ext2_state.block_size;
    ssize_t bytes_read = 0;

    uint8_t* temp_buffer = kmalloc(ext2_state.block_size);
    if (!temp_buffer) {
        return -1;
    }

    for (uint32_t i = start_block; i <= end_block; i++) {
        uint32_t phys_block = ext2_get_block_from_inode(&inode_data, i);
        if (phys_block == 0) {
            break; // Hole in file
        }

        if (!ext2_read_block(phys_block, temp_buffer)) {
            bytes_read = -1;
            break;
        }

        uint32_t to_read = ext2_state.block_size - offset_in_block;
        if (bytes_read + to_read > size) {
            to_read = size - bytes_read;
        }

        memcpy((uint8_t*)buffer + bytes_read, temp_buffer + offset_in_block, to_read);
        bytes_read += to_read;
        offset_in_block = 0;
    }

    kfree(temp_buffer);
    return bytes_read;
}

struct dirent* ext2_readdir(fs_node_t* node, uint64_t index) {
    // ... Implementation needed ...
    return NULL;
}

fs_node_t* ext2_finddir(fs_node_t* node, const char* name) {
    // ... Implementation needed ...
    return NULL;
}

bool ext2_is_initialized(void) {
    return ext2_state.initialized;
}
