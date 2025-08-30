/**
 * Maya OS Process Manager
 * Updated: 2025-08-29 10:58:08 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define MAX_PROCESSES 256
#define PROCESS_STACK_SIZE 16384  // 16 KB

typedef struct {
    process_t* processes[MAX_PROCESSES];
    process_t* current_process;
    uint32_t process_count;
    bool initialized;
} process_manager_t;

static process_manager_t pm;

bool process_init(void) {
    if (pm.initialized) {
        return true;
    }

    memset(&pm, 0, sizeof(process_manager_t));
    pm.initialized = true;
    return true;
}

process_t* process_create(const char* name, process_entry_t entry) {
    if (!pm.initialized || !name || !entry || pm.process_count >= MAX_PROCESSES) {
        return NULL;
    }

    // Allocate process structure
    process_t* process = kmalloc(sizeof(process_t));
    if (!process) {
        return NULL;
    }

    // Initialize process
    memset(process, 0, sizeof(process_t));
    strncpy(process->name, name, PROCESS_NAME_MAX - 1);
    process->name[PROCESS_NAME_MAX - 1] = '\0';
    process->id = pm.process_count++;
    process->state = PROCESS_READY;
    process->entry = entry;

    // Allocate stack
    process->stack = kmalloc(PROCESS_STACK_SIZE);
    if (!process->stack) {
        kfree(process);
        pm.process_count--;
        return NULL;
    }

    // Set up initial stack frame
    uint32_t* stack = (uint32_t*)(process->stack + PROCESS_STACK_SIZE);
    
    // Push initial register values
    *--stack = 0x202;     // EFLAGS (IF set)
    *--stack = 0x08;      // CS
    *--stack = (uint32_t)entry;  // EIP
    *--stack = 0;         // Error code
    *--stack = 0;         // Interrupt number
    *--stack = 0;         // EAX
    *--stack = 0;         // ECX
    *--stack = 0;         // EDX
    *--stack = 0;         // EBX
    *--stack = 0;         // ESP
    *--stack = 0;         // EBP
    *--stack = 0;         // ESI
    *--stack = 0;         // EDI
    *--stack = 0x10;      // DS
    *--stack = 0x10;      // ES
    *--stack = 0x10;      // FS
    *--stack = 0x10;      // GS

    process->esp = (uint32_t)stack;

    // Add to process list
    pm.processes[process->id] = process;

    return process;
}

void process_destroy(process_t* process) {
    if (!pm.initialized || !process) {
        return;
    }

    // Remove from process list
    for (uint32_t i = 0; i < pm.process_count; i++) {
        if (pm.processes[i] == process) {
            // Shift remaining processes
            for (uint32_t j = i; j < pm.process_count - 1; j++) {
                pm.processes[j] = pm.processes[j + 1];
                pm.processes[j]->id = j;
            }
            pm.process_count--;
            break;
        }
    }

    // Free resources
    if (process->stack) {
        kfree(process->stack);
    }
    kfree(process);

    // Update current process if needed
    if (pm.current_process == process) {
        pm.current_process = NULL;
    }
}

void process_schedule(void) {
    if (!pm.initialized || pm.process_count == 0) {
        return;
    }

    // Simple round-robin scheduling
    if (!pm.current_process) {
        pm.current_process = pm.processes[0];
    } else {
        uint32_t next_id = (pm.current_process->id + 1) % pm.process_count;
        pm.current_process = pm.processes[next_id];
    }

    // Switch to next process
    process_switch(pm.current_process);
}

void process_switch(process_t* next) {
    if (!pm.initialized || !next) {
        return;
    }

    // Save current process state
    if (pm.current_process) {
        __asm__ volatile(
            "mov %%esp, %0\n"
            "mov %%ebp, %1\n"
            : "=r"(pm.current_process->esp),
              "=r"(pm.current_process->ebp)
        );
    }

    // Load next process state
    __asm__ volatile(
        "mov %0, %%esp\n"
        "mov %1, %%ebp\n"
        : : "r"(next->esp),
            "r"(next->ebp)
    );

    pm.current_process = next;
}

process_t* process_get_current(void) {
    return pm.current_process;
}

uint32_t process_get_count(void) {
    return pm.process_count;
}

bool process_is_initialized(void) {
    return pm.initialized;
}
