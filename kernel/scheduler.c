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

typedef struct task {
    process_t* process;
    uint32_t quantum_remaining;
    uint32_t total_runtime;
    uint32_t last_run;
    uint8_t priority;
    bool running;
    struct task* next;
} task_t;

static struct {
    task_t* tasks[MAX_TASKS];
    task_t* current_task;
    task_t* idle_task;
    uint32_t task_count;
    uint32_t total_switches;
    bool initialized;
} scheduler_state;

static void scheduler_timer_callback(uint32_t tick_count) {
    if (!scheduler_state.initialized || !scheduler_state.current_task) {
        return;
    }

    // Decrement quantum
    if (scheduler_state.current_task->quantum_remaining > 0) {
        scheduler_state.current_task->quantum_remaining--;
    }

    // If quantum expired, trigger context switch
    if (scheduler_state.current_task->quantum_remaining == 0) {
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
    process_t* idle_process = process_create("idle", scheduler_idle_task);
    if (!idle_process) {
        return false;
    }

    scheduler_state.idle_task = kmalloc(sizeof(task_t));
    if (!scheduler_state.idle_task) {
        process_destroy(idle_process);
        return false;
    }

    scheduler_state.idle_task->process = idle_process;
    scheduler_state.idle_task->quantum_remaining = SCHEDULER_QUANTUM;
    scheduler_state.idle_task->total_runtime = 0;
    scheduler_state.idle_task->last_run = 0;
    scheduler_state.idle_task->priority = 0;
    scheduler_state.idle_task->running = false;
    scheduler_state.idle_task->next = NULL;

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

    // Create new task
    task_t* task = kmalloc(sizeof(task_t));
    if (!task) {
        return false;
    }

    task->process = process;
    task->quantum_remaining = SCHEDULER_QUANTUM;
    task->total_runtime = 0;
    task->last_run = timer_get_ticks();
    task->priority = priority;
    task->running = false;
    task->next = NULL;

    // Add to task list
    scheduler_state.tasks[scheduler_state.task_count++] = task;

    // Sort tasks by priority
    for (uint32_t i = scheduler_state.task_count - 1; i > 0; i--) {
        if (scheduler_state.tasks[i]->priority > scheduler_state.tasks[i-1]->priority) {
            task_t* temp = scheduler_state.tasks[i];
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
        if (scheduler_state.tasks[i]->process == process) {
            // Free task structure
            kfree(scheduler_state.tasks[i]);

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
    if (scheduler_state.current_task) {
        uint32_t current_ticks = timer_get_ticks();
        scheduler_state.current_task->total_runtime += 
            current_ticks - scheduler_state.current_task->last_run;
        scheduler_state.current_task->last_run = current_ticks;
        scheduler_state.current_task->running = false;
    }

    // Find next task to run
    task_t* next_task = NULL;
    for (uint32_t i = 0; i < scheduler_state.task_count; i++) {
        if (!scheduler_state.tasks[i]->running) {
            next_task = scheduler_state.tasks[i];
            break;
        }
    }

    // If no task found, use idle task
    if (!next_task) {
        next_task = scheduler_state.idle_task;
    }

    // Switch to next task
    next_task->quantum_remaining = SCHEDULER_QUANTUM;
    next_task->running = true;
    next_task->last_run = timer_get_ticks();

    scheduler_state.current_task = next_task;
    scheduler_state.total_switches++;

    // Perform context switch
    process_switch(next_task->process);
}

process_t* scheduler_get_current_process(void) {
    if (!scheduler_state.initialized || !scheduler_state.current_task) {
        return NULL;
    }
    return scheduler_state.current_task->process;
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
