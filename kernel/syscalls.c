#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel/kernel.h"
#include "kernel/process.h"

// System call numbers
#define SYS_EXIT    1
#define SYS_FORK    2  
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_WAITPID 7
#define SYS_CREAT   8
#define SYS_LINK    9
#define SYS_UNLINK  10
#define SYS_EXEC    11
#define SYS_CHDIR   12
#define SYS_TIME    13
#define SYS_GETPID  14
#define SYS_SLEEP   15
#define SYS_MALLOC  16
#define SYS_FREE    17

// System call handler
void syscall_handler(struct registers *regs);
void syscall_init(void);

// Individual system call implementations
int sys_exit(int status);
int sys_fork(void);
int sys_read(int fd, void *buf, size_t count);
int sys_write(int fd, const void *buf, size_t count);
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
int sys_getpid(void);
#endif