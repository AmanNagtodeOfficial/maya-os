/**
 * Maya OS Task Scheduler
 * Updated: 2025-08-29 11:13:08 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/scheduler.h"
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/timer.h"
#include "libc/string.h"

#define MAX_TASKS 256
#define SCHEDULER_QUANTUM 10 // milliseconds

static struct {
    process_t* tasks[MAX_TASKS];
    process_t* current_process;
    process_t* idle_process;
    uint32_t task_count;
    uint32_t total_switches;
    bool initialized;
} scheduler_state;

static void scheduler_timer_callback(uint32_t tick_count) {
    if (!scheduler_state.initialized || !scheduler_state.current_process) {
        return;
    }

    // Decrement quantum
    if (scheduler_state.current_process->quantum_remaining > 0) {
        scheduler_state.current_process->quantum_remaining--;
    }

    // If quantum expired, trigger context switch
    if (scheduler_state.current_process->quantum_remaining == 0) {
        scheduler_switch_task();
    }
}

static void scheduler_idle_task(void) {
    while (1) {
        __asm__ volatile("hlt");
    }
}

bool scheduler_init(void) {
    if (scheduler_state.initialized) {
        return true;
    }

    // Initialize state
    memset(&scheduler_state, 0, sizeof(scheduler_state));

    // Create idle task
    process_t* idle = process_create("idle", (void*)scheduler_idle_task);
    if (!idle) {
        return false;
    }

    scheduler_state.idle_process = idle;
    scheduler_state.idle_process->quantum_remaining = SCHEDULER_QUANTUM;
    scheduler_state.idle_process->priority = 0;
    scheduler_state.idle_process->state = PROCESS_STATE_READY;

    // Register timer callback
    timer_set_callback(scheduler_timer_callback);

    scheduler_state.initialized = true;
    return true;
}

bool scheduler_add_task(process_t* process, uint8_t priority) {
    if (!scheduler_state.initialized || !process || 
        scheduler_state.task_count >= MAX_TASKS) {
        return false;
    }

    process->quantum_remaining = SCHEDULER_QUANTUM;
    process->total_runtime = 0;
    process->last_run = timer_get_ticks();
    process->priority = priority;
    process->state = PROCESS_STATE_READY;

    // Add to task list
    scheduler_state.tasks[scheduler_state.task_count++] = process;

    // Sort tasks by priority
    for (uint32_t i = scheduler_state.task_count - 1; i > 0; i--) {
        if (scheduler_state.tasks[i]->priority > scheduler_state.tasks[i-1]->priority) {
            process_t* temp = scheduler_state.tasks[i];
            scheduler_state.tasks[i] = scheduler_state.tasks[i-1];
            scheduler_state.tasks[i-1] = temp;
        }
    }

    return true;
}

void scheduler_remove_task(process_t* process) {
    if (!scheduler_state.initialized || !process) {
        return;
    }

    // Find and remove task
    for (uint32_t i = 0; i < scheduler_state.task_count; i++) {
        if (scheduler_state.tasks[i] == process) {
            // Shift remaining tasks
            for (uint32_t j = i; j < scheduler_state.task_count - 1; j++) {
                scheduler_state.tasks[j] = scheduler_state.tasks[j + 1];
            }

            scheduler_state.task_count--;
            break;
        }
    }
}

void scheduler_switch_task(void) {
    if (!scheduler_state.initialized || scheduler_state.task_count == 0) {
        return;
    }

    // Update current task statistics
    if (scheduler_state.current_process) {
        uint32_t current_ticks = timer_get_ticks();
        scheduler_state.current_process->total_runtime += 
            current_ticks - scheduler_state.current_process->last_run;
        scheduler_state.current_process->last_run = current_ticks;
        scheduler_state.current_process->state = PROCESS_STATE_READY;
    }

    // Find next task to run
    process_t* next_process = NULL;
    for (uint32_t i = 0; i < scheduler_state.task_count; i++) {
        if (scheduler_state.tasks[i]->state == PROCESS_STATE_READY) {
            next_process = scheduler_state.tasks[i];
            break;
        }
    }

    // If no task found, use idle task
    if (!next_process) {
        next_process = scheduler_state.idle_process;
    }

    // Switch to next task
    next_process->quantum_remaining = SCHEDULER_QUANTUM;
    next_process->state = PROCESS_STATE_RUNNING;
    next_process->last_run = timer_get_ticks();

    scheduler_state.current_process = next_process;
    scheduler_state.total_switches++;

    // Perform context switch
    process_switch(next_process);
}

process_t* scheduler_get_current_process(void) {
    if (!scheduler_state.initialized || !scheduler_state.current_process) {
        return NULL;
    }
    return scheduler_state.current_process;
}

uint32_t scheduler_get_task_count(void) {
    return scheduler_state.task_count;
}

uint32_t scheduler_get_total_switches(void) {
    return scheduler_state.total_switches;
}

bool scheduler_is_initialized(void) {
    return scheduler_state.initialized;
}
