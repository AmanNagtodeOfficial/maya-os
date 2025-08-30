/**
 * Maya OS Mutex Implementation
 * Updated: 2025-08-29 11:16:18 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/mutex.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

typedef struct mutex_waiter {
    process_t* process;
    struct mutex_waiter* next;
} mutex_waiter_t;

static bool mutex_try_lock(mutex_t* mutex) {
    uint32_t old_value = __sync_val_compare_and_swap(&mutex->locked, 0, 1);
    if (old_value == 0) {
        mutex->owner = process_get_current();
        return true;
    }
    return false;
}

void mutex_init(mutex_t* mutex) {
    if (!mutex) {
        return;
    }

    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->waiters = NULL;
}

void mutex_lock(mutex_t* mutex) {
    if (!mutex) {
        return;
    }

    // Disable interrupts while manipulating mutex state
    uint32_t flags = interrupt_disable();

    // Try to acquire lock
    if (mutex_try_lock(mutex)) {
        interrupt_restore(flags);
        return;
    }

    // If already owned by current process, deadlock
    process_t* current = process_get_current();
    if (mutex->owner == current) {
        // TODO: Handle deadlock detection
        interrupt_restore(flags);
        return;
    }

    // Create waiter entry
    mutex_waiter_t* waiter = kmalloc(sizeof(mutex_waiter_t));
    if (!waiter) {
        interrupt_restore(flags);
        return;
    }

    waiter->process = current;
    waiter->next = NULL;

    // Add to waiter list
    if (!mutex->waiters) {
        mutex->waiters = waiter;
    } else {
        mutex_waiter_t* last = mutex->waiters;
        while (last->next) {
            last = last->next;
        }
        last->next = waiter;
    }

    while (!mutex_try_lock(mutex)) {
        // Re-enable interrupts while waiting
        interrupt_restore(flags);
        
        // Yield to other processes
        scheduler_switch_task();

        // Disable interrupts to check mutex again
        flags = interrupt_disable();
    }

    // Remove from waiter list
    if (mutex->waiters == waiter) {
        mutex->waiters = waiter->next;
    } else {
        mutex_waiter_t* prev = mutex->waiters;
        while (prev->next != waiter) {
            prev = prev->next;
        }
        prev->next = waiter->next;
    }

    kfree(waiter);
    interrupt_restore(flags);
}

bool mutex_try_lock(mutex_t* mutex) {
    if (!mutex) {
        return false;
    }

    uint32_t flags = interrupt_disable();
    bool result = mutex_try_lock(mutex);
    interrupt_restore(flags);

    return result;
}

void mutex_unlock(mutex_t* mutex) {
    if (!mutex) {
        return;
    }

    uint32_t flags = interrupt_disable();

    // Verify owner
    process_t* current = process_get_current();
    if (mutex->owner != current) {
        interrupt_restore(flags);
        return;
    }

    mutex->owner = NULL;
    __sync_synchronize();
    mutex->locked = 0;

    // Wake up first waiter
    if (mutex->waiters) {
        process_wake(mutex->waiters->process);
    }

    interrupt_restore(flags);
}

bool mutex_is_locked(mutex_t* mutex) {
    if (!mutex) {
        return false;
    }
    return mutex->locked != 0;
}

process_t* mutex_get_owner(mutex_t* mutex) {
    if (!mutex) {
        return NULL;
    }
    return mutex->owner;
}
