
#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_MAX_PATH 256
#define VFS_MAX_FILENAME 64
#define VFS_MAX_MOUNTPOINTS 16
#define VFS_MAX_OPEN_FILES 256

// File types
#define VFS_FILE_TYPE_REGULAR 0
#define VFS_FILE_TYPE_DIRECTORY 1
#define VFS_FILE_TYPE_SYMLINK 2
#define VFS_FILE_TYPE_DEVICE 3
#define VFS_FILE_TYPE_FIFO 4
#define VFS_FILE_TYPE_SOCKET 5

// File access modes
#define VFS_O_RDONLY 0x00
#define VFS_O_WRONLY 0x01
#define VFS_O_RDWR   0x02
#define VFS_O_CREAT  0x04
#define VFS_O_EXCL   0x08
#define VFS_O_TRUNC  0x10
#define VFS_O_APPEND 0x20

// Seek modes
#define VFS_SEEK_SET 0
#define VFS_SEEK_CUR 1
#define VFS_SEEK_END 2

// File permissions
#define VFS_PERM_READ  0x04
#define VFS_PERM_WRITE 0x02
#define VFS_PERM_EXEC  0x01

// Error codes
#define VFS_OK 0
#define VFS_ERROR_NOT_FOUND -1
#define VFS_ERROR_ACCESS_DENIED -2
#define VFS_ERROR_INVALID_PATH -3
#define VFS_ERROR_FILE_EXISTS -4
#define VFS_ERROR_NOT_A_FILE -5
#define VFS_ERROR_NOT_A_DIRECTORY -6
#define VFS_ERROR_INVALID_HANDLE -7
#define VFS_ERROR_IO_ERROR -8
#define VFS_ERROR_NO_SPACE -9
#define VFS_ERROR_INVALID_OPERATION -10

// Forward declarations
typedef struct vfs_node vfs_node_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_filesystem vfs_filesystem_t;
typedef struct vfs_mountpoint vfs_mountpoint_t;

// File statistics structure
typedef struct {
    uint32_t size;
    uint32_t type;
    uint32_t permissions;
    uint32_t uid;
    uint32_t gid;
    uint32_t atime;  // Access time
    uint32_t mtime;  // Modification time
    uint32_t ctime;  // Creation time
    uint32_t blocks;
    uint32_t block_size;
} vfs_stat_t;

// Directory entry structure
typedef struct {
    char name[VFS_MAX_FILENAME];
    uint32_t type;
    uint32_t size;
    uint32_t inode;
} vfs_dirent_t;

// File system operations structure
typedef struct {
    // File operations
    int (*open)(vfs_node_t *node, uint32_t flags);
    int (*close)(vfs_file_t *file);
    int (*read)(vfs_file_t *file, void *buffer, size_t size, uint32_t offset);
    int (*write)(vfs_file_t *file, const void *buffer, size_t size, uint32_t offset);
    int (*ioctl)(vfs_file_t *file, uint32_t request, void *arg);
    
    // Directory operations
    int (*readdir)(vfs_node_t *node, uint32_t index, vfs_dirent_t *dirent);
    vfs_node_t *(*finddir)(vfs_node_t *node, const char *name);
    int (*mkdir)(vfs_node_t *parent, const char *name, uint32_t permissions);
    int (*rmdir)(vfs_node_t *node);
    
    // Node operations
    int (*create)(vfs_node_t *parent, const char *name, uint32_t permissions);
    int (*unlink)(vfs_node_t *node);
    int (*rename)(vfs_node_t *node, const char *new_name);
    int (*stat)(vfs_node_t *node, vfs_stat_t *stat);
    int (*chmod)(vfs_node_t *node, uint32_t permissions);
    int (*chown)(vfs_node_t *node, uint32_t uid, uint32_t gid);
    
    // File system operations
    int (*sync)(vfs_filesystem_t *fs);
    int (*statfs)(vfs_filesystem_t *fs, vfs_stat_t *stat);
} vfs_operations_t;

// VFS node structure (inode equivalent)
struct vfs_node {
    char name[VFS_MAX_FILENAME];
    uint32_t type;              // File type
    uint32_t permissions;       // File permissions
    uint32_t uid, gid;         // Owner and group
    uint32_t size;             // File size
    uint32_t atime, mtime, ctime; // Timestamps
    uint32_t inode;            // Inode number
    uint32_t ref_count;        // Reference count
    
    vfs_operations_t *ops;     // Operations for this node
    vfs_filesystem_t *fs;      // File system this node belongs to
    void *fs_data;             // File system specific data
    
    struct vfs_node *parent;   // Parent directory
    struct vfs_node *next;     // Next sibling
    struct vfs_node *children; // Children (for directories)
};

// Open file structure
struct vfs_file {
    vfs_node_t *node;          // Associated node
    uint32_t flags;            // Open flags
    uint32_t position;         // Current file position
    uint32_t ref_count;        // Reference count
    void *private_data;        // File-specific data
};

// File system structure
struct vfs_filesystem {
    char name[32];             // File system name
    vfs_operations_t *ops;     // File system operations
    void *private_data;        // File system specific data
    uint32_t block_size;       // Block size
    uint32_t total_blocks;     // Total blocks
    uint32_t free_blocks;      // Free blocks
};

// Mount point structure
struct vfs_mountpoint {
    char path[VFS_MAX_PATH];   // Mount path
    vfs_filesystem_t *fs;      // Mounted file system
    vfs_node_t *root;          // Root node of mounted fs
    uint32_t flags;            // Mount flags
    struct vfs_mountpoint *next; // Next mount point
};

// VFS initialization and management
void vfs_init(void);
void vfs_shutdown(void);

// Mount point management
int vfs_mount(const char *path, vfs_filesystem_t *fs, uint32_t flags);
int vfs_unmount(const char *path);
vfs_mountpoint_t *vfs_find_mountpoint(const char *path);

// File operations
int vfs_open(const char *path, uint32_t flags);
int vfs_close(int fd);
int vfs_read(int fd, void *buffer, size_t size);
int vfs_write(int fd, const void *buffer, size_t size);
int vfs_seek(int fd, long offset, int whence);
long vfs_tell(int fd);
int vfs_eof(int fd);
int vfs_flush(int fd);

// File management
int vfs_create(const char *path, uint32_t permissions);
int vfs_unlink(const char *path);
int vfs_rename(const char *old_path, const char *new_path);
int vfs_stat(const char *path, vfs_stat_t *stat);
int vfs_fstat(int fd, vfs_stat_t *stat);
int vfs_chmod(const char *path, uint32_t permissions);
int vfs_chown(const char *path, uint32_t uid, uint32_t gid);

// Directory operations
int vfs_mkdir(const char *path, uint32_t permissions);
int vfs_rmdir(const char *path);
int vfs_opendir(const char *path);
int vfs_readdir(int dir_fd, vfs_dirent_t *dirent);
int vfs_closedir(int dir_fd);
int vfs_chdir(const char *path);
char *vfs_getcwd(char *buffer, size_t size);

// Path operations
int vfs_resolve_path(const char *path, char *resolved_path);
int vfs_is_absolute_path(const char *path);
char *vfs_get_filename(const char *path);
char *vfs_get_dirname(const char *path);
int vfs_split_path(const char *path, char *dirname, char *basenam