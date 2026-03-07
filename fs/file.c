/**
 * Maya OS File Descriptor Management
 * Updated: 2026-03-07 20:58:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "fs/file.h"
#include "kernel/process.h"
#include "kernel/memory.h"
#include "libc/string.h"

int fd_alloc(vfs_node_t *node, uint32_t flags) {
    process_t *current = process_get_current();
    if (!current) return -1;

    for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
        if (current->fd_table.descriptors[i].node == NULL) {
            current->fd_table.descriptors[i].node = node;
            current->fd_table.descriptors[i].offset = 0;
            current->fd_table.descriptors[i].flags = flags;
            return i;
        }
    }
    return -1;
}

void fd_free(int fd) {
    process_t *current = process_get_current();
    if (!current || fd < 0 || fd >= MAX_FILES_PER_PROCESS) return;

    current->fd_table.descriptors[fd].node = NULL;
    current->fd_table.descriptors[fd].offset = 0;
}

file_descriptor_t *fd_get(int fd) {
    process_t *current = process_get_current();
    if (!current || fd < 0 || fd >= MAX_FILES_PER_PROCESS) return NULL;

    if (current->fd_table.descriptors[fd].node == NULL) return NULL;
    return &current->fd_table.descriptors[fd];
}
