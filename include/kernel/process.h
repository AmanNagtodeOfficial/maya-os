#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 256

typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

#include "fs/file.h"

typedef struct process {
    uint32_t pid;
    process_state_t state;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t page_directory;
    
    // Scheduling fields
    uint8_t priority;
    uint32_t quantum_remaining;
    uint32_t total_runtime;
    uint32_t last_run;
    
    // Filesystem and Security
    fd_table_t fd_table;
    uint32_t caps;
    
    struct process *next;
    char name[256];
} __attribute__((packed)) process_t;

// Process management
void process_init(void);
uint32_t process_create(void (*entry_point)(void), const char *name);
void process_kill(uint32_t pid);
void process_switch(void);
struct process *process_get_current(void);
void schedule(void);

// System calls
void syscall_init(void);

#endif



