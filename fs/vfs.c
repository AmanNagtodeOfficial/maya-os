/**
 * Maya OS Virtual File System (VFS)
 * Updated: 2026-03-07 20:55:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "fs/vfs.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "kernel/logging.h"

vfs_node_t *vfs_root = NULL;

void vfs_init(void) {
    vfs_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    memset(vfs_root, 0, sizeof(vfs_node_t));
    strcpy(vfs_root->name, "/");
    vfs_root->flags = VFS_DIRECTORY | VFS_MOUNTPOINT;
    
    KLOG_I("VFS initialized with root mount point.");
}

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write) {
    (void)read; (void)write;
    if (node && node->open) {
        node->open(node);
    }
}

void vfs_close(vfs_node_t *node) {
    if (node && node->close) {
        node->close(node);
    }
}

struct vfs_dirent *vfs_readdir(vfs_node_t *node, uint32_t index) {
    if (node && (node->flags & 0x07) == VFS_DIRECTORY && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

vfs_node_t *vfs_finddir(vfs_node_t *node, char *name) {
    if (node && (node->flags & 0x07) == VFS_DIRECTORY && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}
