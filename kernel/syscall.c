/**
 * Maya OS System Call Handler
 * Updated: 2026-03-07 20:20:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/syscall.h"
#include "kernel/interrupts.h"
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "drivers/vga.h"
#include "libc/string.h"
#include "libc/stdio.h"

#define MAX_SYSCALLS 128
#define SYSCALL_INT 0x80

typedef struct {
    syscall_handler_t handler;
    const char* name;
    uint32_t arg_count;
} syscall_entry_t;

static struct {
    syscall_entry_t syscalls[MAX_SYSCALLS];
    uint32_t syscall_count;
    bool initialized;
} syscall_state;

static void syscall_handler(struct registers *r) {
    uint32_t syscall_num = r->eax;
    
    if (syscall_num >= syscall_state.syscall_count || 
        !syscall_state.syscalls[syscall_num].handler) {
        r->eax = (uint32_t)-1;
        KLOG_W("Invalid syscall: %d", syscall_num);
        return;
    }

    // Get arguments from registers
    uint32_t args[] = {r->ebx, r->ecx, r->edx, r->esi, r->edi};
    
    // Call handler
    r->eax = syscall_state.syscalls[syscall_num].handler(args, 
                syscall_state.syscalls[syscall_num].arg_count);
}

bool syscall_init(void) {
    if (syscall_state.initialized) {
        return true;
    }

    memset(&syscall_state, 0, sizeof(syscall_state));

    // Register interrupt handler
    register_interrupt_handler(SYSCALL_INT, (isr_t)syscall_handler);

    syscall_init_defaults();

    syscall_state.initialized = true;
    KLOG_I("System call subsystem initialized.");
    return true;
}

bool syscall_register(uint32_t num, syscall_handler_t handler, 
                     const char* name, uint32_t arg_count) {
    if (!syscall_state.initialized || num >= MAX_SYSCALLS || !handler || !name) {
        return false;
    }

    syscall_state.syscalls[num].handler = handler;
    syscall_state.syscalls[num].name = name;
    syscall_state.syscalls[num].arg_count = arg_count;

    if (num >= syscall_state.syscall_count) {
        syscall_state.syscall_count = num + 1;
    }

    return true;
}

// ─── System Call Handlers ───────────────────────────────────────────────────

static uint32_t sys_exit(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 1) return -1;
    int code = (int)args[0];
    KLOG_I("Process exit requested with code: %d", code);
    process_t* current = process_get_current();
    if (current) {
        process_destroy(current);
    }
    return 0;
}

static uint32_t sys_write(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 3) return -1;
    int fd = args[0];
    const char* buf = (const char*)args[1];
    size_t count = args[2];

    if (!buf || count == 0 || !memory_validate_user_buffer(buf, count)) return -1;

    if (fd == 1 || fd == 2) { // stdout or stderr
        vga_write(buf, count);
        return count;
    }

    // TODO: Implement filesystem write via VFS
    return -1;
}

static uint32_t sys_read(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 3) return -1;
    int fd = args[0];
    void* buf = (void*)args[1];
    size_t count = args[2];

    if (!buf || count == 0 || !memory_validate_user_buffer(buf, count)) return -1;

    if (fd == 0) { // stdin
        // TODO: Read from keyboard driver ring buffer
        return 0;
    }

    // TODO: Implement filesystem read via VFS
    return -1;
}

static uint32_t sys_open(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 2) return -1;
    const char* path = (const char*)args[0];
    // int flags = args[1]; // unused for now

    if (!path || !memory_validate_user_string(path)) return -1;

    // TODO: Implement vfs_open
    return -1;
}

static uint32_t sys_close(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 1) return -1;
    // int fd = args[0];
    // TODO: Implement vfs_close
    return -1;
}

static uint32_t sys_getpid(uint32_t args[], uint32_t arg_count) {
    process_t* current = process_get_current();
    return current ? current->pid : (uint32_t)-1;
}

static uint32_t sys_sleep(uint32_t args[], uint32_t arg_count) {
    if (arg_count < 1) return -1;
    uint32_t ms = args[0];
    // TODO: Implement process_sleep linked to PIT/APIC timer
    (void)ms;
    return 0;
}

static uint32_t sys_fork(uint32_t args[], uint32_t arg_count) {
    // TODO: Implement process_clone/fork
    return -1;
}

void syscall_init_defaults(void) {
    syscall_register(0, sys_exit,   "exit",   1);
    syscall_register(1, sys_write,  "write",  3);
    syscall_register(2, sys_read,   "read",   3);
    syscall_register(3, sys_open,   "open",   2);
    syscall_register(4, sys_close,  "close",  1);
    syscall_register(5, sys_getpid, "getpid", 0);
    syscall_register(6, sys_sleep,  "sleep",  1);
    syscall_register(7, sys_fork,   "fork",   0);
}

const char* syscall_get_name(uint32_t num) {
    if (num >= syscall_state.syscall_count) return NULL;
    return syscall_state.syscalls[num].name;
}

uint32_t syscall_get_count(void) {
    return syscall_state.syscall_count;
}

bool syscall_is_initialized(void) {
    return syscall_state.initialized;
}

