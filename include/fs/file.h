#ifndef FILE_H
#define FILE_H

#include "fs/vfs.h"

#define MAX_FILES_PER_PROCESS 32

typedef struct {
    vfs_node_t *node;
    uint32_t offset;
    uint32_t flags;
} file_descriptor_t;

typedef struct {
    file_descriptor_t descriptors[MAX_FILES_PER_PROCESS];
    uint32_t count;
} fd_table_t;

int fd_alloc(vfs_node_t *node, uint32_t flags);
void fd_free(int fd);
file_descriptor_t *fd_get(int fd);

#endif
