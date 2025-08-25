#ifndef SYSCALLS_H
#define SYSCALLS_H

#define MAX_SYSCALLS 256

typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

extern syscall_handler_t syscall_table[MAX_SYSCALLS];

void syscall_init(void);
void syscall_handler(struct registers *regs);

#endif