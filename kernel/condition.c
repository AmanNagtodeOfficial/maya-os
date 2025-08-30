/**
 * Maya OS Condition Variable Implementation
 * Updated: 2025-08-29 11:18:08 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/condition.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

typedef struct condition_waiter {
    process_t* process;
    struct condition_waiter* next;
} condition_waiter_t;

void condition_init(condition_t* cond) {
    if (!cond) {
        return;
    }

    cond->waiters = NULL;
    spinlock_init(&cond->lock);
}

void condition_wait(condition_t* cond, mutex_t* mutex) {
    if (!cond || !mutex) {
        return;
    }

    spinlock_acquire(&cond->lock);

    // Create waiter entry
    process_t* current = process_get_current();
    condition_waiter_t* waiter = kmalloc(sizeof(condition_waiter_t));
    if (!waiter) {
        spinlock_release(&cond->lock);
        return;
    }

    waiter->process = current;
    waiter->next = NULL;

    // Add to waiter list
    if (!cond->waiters) {
        cond->waiters = waiter;
    } else {
        condition_waiter_t* last = cond->waiters;
        while (last->next) {
            last = last->next;
        }
        last->next = waiter;
    }

    // Release mutex and lock
    mutex_unlock(mutex);
    spinlock_release(&cond->lock);

    // Block process
    process_block(current);

    // Reacquire mutex when woken up
    mutex_lock(mutex);
}

void condition_signal(condition_t* cond) {
    if (!cond) {
        return;
    }

    spinlock_acquire(&cond->lock);

    // Wake up first waiter
    if (cond->waiters) {
        condition_waiter_t* waiter = cond->waiters;
        cond->waiters = waiter->next;

        process_wake(waiter->process);
        kfree(waiter);
    }

    spinlock_release(&cond->lock);
}

void condition_broadcast(condition_t* cond) {
    if (!cond) {
        return;
    }

    spinlock_acquire(&cond->lock);

    // Wake up all waiters
    while (cond->waiters) {
        condition_waiter_t* waiter = cond->waiters;
        cond->waiters = waiter->next;

        process_wake(waiter->process);
        kfree(waiter);
    }

    spinlock_release(&cond->lock);
}

void condition_destroy(condition_t* cond) {
    if (!cond) {
        return;
    }

    spinlock_acquire(&cond->lock);

    // Clean up remaining waiters
    while (cond->waiters) {
        condition_waiter_t* waiter = cond->waiters;
        cond->waiters = waiter->next;

        process_wake(waiter->process);
        kfree(waiter);
    }

    spinlock_release(&cond->lock);
}

bool condition_has_waiters(condition_t* cond) {
    if (!cond) {
        return false;
    }

    spinlock_acquire(&cond->lock);
    bool has_waiters = cond->waiters != NULL;
    spinlock_release(&cond->lock);

    return has_waiters;
}
