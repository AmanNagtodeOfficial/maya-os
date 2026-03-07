#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEVICE 0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE      0x05
#define VFS_SYMLINK   0x06
#define VFS_MOUNTPOINT 0x08

struct vfs_node;

typedef uint32_t (*vfs_read_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*vfs_write_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*vfs_open_t)(struct vfs_node*);
typedef void (*vfs_close_t)(struct vfs_node*);
typedef struct vfs_dirent* (*vfs_readdir_t)(struct vfs_node*, uint32_t);
typedef struct vfs_node* (*vfs_finddir_t)(struct vfs_node*, char *name);

typedef struct vfs_node {
    char name[128];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    vfs_read_t read;
    vfs_write_t write;
    vfs_open_t open;
    vfs_close_t close;
    vfs_readdir_t readdir;
    vfs_finddir_t finddir;
    struct vfs_node *ptr; // For mountpoints
} vfs_node_t;

struct vfs_dirent {
    char name[128];
    uint32_t inode;
};

extern vfs_node_t *vfs_root;

void vfs_init(void);
uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write);
void vfs_close(vfs_node_t *node);
struct vfs_dirent *vfs_readdir(vfs_node_t *node, uint32_t index);
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name);

#endif