/**
 * Maya OS System Call Table Implementation
 * Updated: 2025-08-29 11:20:30 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/syscall_table.h"
#include "kernel/syscall.h"
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/fs.h"
#include "libc/string.h"

// System call handler prototypes
static uint32_t sys_exit(uint32_t args[]);
static uint32_t sys_fork(uint32_t args[]);
static uint32_t sys_read(uint32_t args[]);
static uint32_t sys_write(uint32_t args[]);
static uint32_t sys_open(uint32_t args[]);
static uint32_t sys_close(uint32_t args[]);
static uint32_t sys_execve(uint32_t args[]);
static uint32_t sys_wait(uint32_t args[]);
static uint32_t sys_pipe(uint32_t args[]);
static uint32_t sys_dup(uint32_t args[]);
static uint32_t sys_dup2(uint32_t args[]);
static uint32_t sys_getpid(uint32_t args[]);
static uint32_t sys_brk(uint32_t args[]);
static uint32_t sys_sleep(uint32_t args[]);
static uint32_t sys_kill(uint32_t args[]);
static uint32_t sys_socket(uint32_t args[]);
static uint32_t sys_bind(uint32_t args[]);
static uint32_t sys_connect(uint32_t args[]);
static uint32_t sys_listen(uint32_t args[]);
static uint32_t sys_accept(uint32_t args[]);
static uint32_t sys_send(uint32_t args[]);
static uint32_t sys_recv(uint32_t args[]);

// System call table
const syscall_entry_t syscall_table[] = {
    [SYS_EXIT]    = {"exit",    sys_exit,    1},
    [SYS_FORK]    = {"fork",    sys_fork,    0},
    [SYS_READ]    = {"read",    sys_read,    3},
    [SYS_WRITE]   = {"write",   sys_write,   3},
    [SYS_OPEN]    = {"open",    sys_open,    3},
    [SYS_CLOSE]   = {"close",   sys_close,   1},
    [SYS_EXECVE]  = {"execve",  sys_execve,  3},
    [SYS_WAIT]    = {"wait",    sys_wait,    1},
    [SYS_PIPE]    = {"pipe",    sys_pipe,    1},
    [SYS_DUP]     = {"dup",     sys_dup,     1},
    [SYS_DUP2]    = {"dup2",    sys_dup2,    2},
    [SYS_GETPID]  = {"getpid",  sys_getpid,  0},
    [SYS_BRK]     = {"brk",     sys_brk,     1},
    [SYS_SLEEP]   = {"sleep",   sys_sleep,   1},
    [SYS_KILL]    = {"kill",    sys_kill,    2},
    [SYS_SOCKET]  = {"socket",  sys_socket,  3},
    [SYS_BIND]    = {"bind",    sys_bind,    3},
    [SYS_CONNECT] = {"connect", sys_connect, 3},
    [SYS_LISTEN]  = {"listen",  sys_listen,  2},
    [SYS_ACCEPT]  = {"accept",  sys_accept,  3},
    [SYS_SEND]    = {"send",    sys_send,    4},
    [SYS_RECV]    = {"recv",    sys_recv,    4},
};

const size_t syscall_table_size = sizeof(syscall_table) / sizeof(syscall_entry_t);

// System call implementations
static uint32_t sys_exit(uint32_t args[]) {
    process_t* current = process_get_current();
    process_exit(current, args[0]);
    return 0;
}

static uint32_t sys_fork(uint32_t args[]) {
    process_t* current = process_get_current();
    process_t* child = process_fork(current);
    return child ? child->pid : -1;
}

static uint32_t sys_read(uint32_t args[]) {
    int fd = args[0];
    void* buf = (void*)args[1];
    size_t count = args[2];

    if (!memory_validate_user_buffer(buf, count)) {
        return -1;
    }

    return fs_read(fd, buf, count);
}

static uint32_t sys_write(uint32_t args[]) {
    int fd = args[0];
    const void* buf = (const void*)args[1];
    size_t count = args[2];

    if (!memory_validate_user_buffer((void*)buf, count)) {
        return -1;
    }

    return fs_write(fd, buf, count);
}

static uint32_t sys_open(uint32_t args[]) {
    const char* path = (const char*)args[0];
    int flags = args[1];
    mode_t mode = args[2];

    if (!memory_validate_user_string(path)) {
        return -1;
    }

    return fs_open(path, flags, mode);
}

static uint32_t sys_close(uint32_t args[]) {
    return fs_close(args[0]);
}

static uint32_t sys_execve(uint32_t args[]) {
    const char* path = (const char*)args[0];
    char* const* argv = (char* const*)args[1];
    char* const* envp = (char* const*)args[2];

    if (!memory_validate_user_string(path) ||
        !memory_validate_user_ptr(argv) ||
        !memory_validate_user_ptr(envp)) {
        return -1;
    }

    return process_execve(path, argv, envp);
}

static uint32_t sys_wait(uint32_t args[]) {
    int* status = (int*)args[0];

    if (status && !memory_validate_user_buffer(status, sizeof(int))) {
        return -1;
    }

    return process_wait(status);
}

static uint32_t sys_pipe(uint32_t args[]) {
    int* fds = (int*)args[0];

    if (!memory_validate_user_buffer(fds, 2 * sizeof(int))) {
        return -1;
    }

    return pipe_create(&fds[0], &fds[1]);
}

static uint32_t sys_dup(uint32_t args[]) {
    return fs_dup(args[0]);
}

static uint32_t sys_dup2(uint32_t args[]) {
    return fs_dup2(args[0], args[1]);
}

static uint32_t sys_getpid(uint32_t args[]) {
    process_t* current = process_get_current();
    return current ? current->pid : -1;
}

static uint32_t sys_brk(uint32_t args[]) {
    return memory_sbrk(args[0]);
}

static uint32_t sys_sleep(uint32_t args[]) {
    timer_sleep(args[0]);
    return 0;
}

static uint32_t sys_kill(uint32_t args[]) {
    return process_kill(args[0], args[1]);
}

static uint32_t sys_socket(uint32_t args[]) {
    int domain = args[0];
    int type = args[1];
    int protocol = args[2];
    return net_socket(domain, type, protocol);
}

static uint32_t sys_bind(uint32_t args[]) {
    int sockfd = args[0];
    const struct sockaddr* addr = (const struct sockaddr*)args[1];
    socklen_t addrlen = args[2];

    if (!memory_validate_user_buffer((void*)addr, addrlen)) {
        return -1;
    }

    return net_bind(sockfd, addr, addrlen);
}

static uint32_t sys_connect(uint32_t args[]) {
    int sockfd = args[0];
    const struct sockaddr* addr = (const struct sockaddr*)args[1];
    socklen_t addrlen = args[2];

    if (!memory_validate_user_buffer((void*)addr, addrlen)) {
        return -1;
    }

    return net_connect(sockfd, addr, addrlen);
}

static uint32_t sys_listen(uint32_t args[]) {
    return net_listen(args[0], args[1]);
}

static uint32_t sys_accept(uint32_t args[]) {
    int sockfd = args[0];
    struct sockaddr* addr = (struct sockaddr*)args[1];
    socklen_t* addrlen = (socklen_t*)args[2];

    if (addr && addrlen && 
        !memory_validate_user_buffer(addr, *addrlen) ||
        !memory_validate_user_buffer(addrlen, sizeof(socklen_t))) {
        return -1;
    }

    return net_accept(sockfd, addr, addrlen);
}

static uint32_t sys_send(uint32_t args[]) {
    int sockfd = args[0];
    const void* buf = (const void*)args[1];
    size_t len = args[2];
    int flags = args[3];

    if (!memory_validate_user_buffer((void*)buf, len)) {
        return -1;
    }

    return net_send(sockfd, buf, len, flags);
}

static uint32_t sys_recv(uint32_t args[]) {
    int sockfd = args[0];
    void* buf = (void*)args[1];
    size_t len = args[2];
    int flags = args[3];

    if (!memory_validate_user_buffer(buf, len)) {
        return -1;
    }

    return net_recv(sockfd, buf, len, flags);
}
