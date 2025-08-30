/**
 * Maya OS Semaphore Implementation
 * Updated: 2025-08-29 11:17:03 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/semaphore.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

typedef struct semaphore_waiter {
    process_t* process;
    struct semaphore_waiter* next;
} semaphore_waiter_t;

void semaphore_init(semaphore_t* sem, int32_t value) {
    if (!sem || value < 0) {
        return;
    }

    sem->value = value;
    sem->waiters = NULL;
    spinlock_init(&sem->lock);
}

void semaphore_wait(semaphore_t* sem) {
    if (!sem) {
        return;
    }

    spinlock_acquire(&sem->lock);

    if (sem->value > 0) {
        sem->value--;
        spinlock_release(&sem->lock);
        return;
    }

    // Create waiter entry
    process_t* current = process_get_current();
    semaphore_waiter_t* waiter = kmalloc(sizeof(semaphore_waiter_t));
    if (!waiter) {
        spinlock_release(&sem->lock);
        return;
    }

    waiter->process = current;
    waiter->next = NULL;

    // Add to waiter list
    if (!sem->waiters) {
        sem->waiters = waiter;
    } else {
        semaphore_waiter_t* last = sem->waiters;
        while (last->next) {
            last = last->next;
        }
        last->next = waiter;
    }

    // Release lock and block process
    spinlock_release(&sem->lock);
    process_block(current);

    // When woken up, clean up waiter entry
    spinlock_acquire(&sem->lock);

    // Remove from waiter list
    if (sem->waiters == waiter) {
        sem->waiters = waiter->next;
    } else {
        semaphore_waiter_t* prev = sem->waiters;
        while (prev->next != waiter) {
            prev = prev->next;
        }
        prev->next = waiter->next;
    }

    kfree(waiter);
    spinlock_release(&sem->lock);
}

bool semaphore_try_wait(semaphore_t* sem) {
    if (!sem) {
        return false;
    }

    spinlock_acquire(&sem->lock);

    if (sem->value > 0) {
        sem->value--;
        spinlock_release(&sem->lock);
        return true;
    }

    spinlock_release(&sem->lock);
    return false;
}

void semaphore_signal(semaphore_t* sem) {
    if (!sem) {
        return;
    }

    spinlock_acquire(&sem->lock);

    sem->value++;

    // Wake up first waiter
    if (sem->waiters) {
        process_wake(sem->waiters->process);
    }

    spinlock_release(&sem->lock);
}

int32_t semaphore_get_value(semaphore_t* sem) {
    if (!sem) {
        return 0;
    }

    spinlock_acquire(&sem->lock);
    int32_t value = sem->value;
    spinlock_release(&sem->lock);

    return value;
}

void semaphore_destroy(semaphore_t* sem) {
    if (!sem) {
        return;
    }

    spinlock_acquire(&sem->lock);

    // Wake up all waiters
    semaphore_waiter_t* waiter = sem->waiters;
    while (waiter) {
        process_wake(waiter->process);
        semaphore_waiter_t* next = waiter->next;
        kfree(waiter);
        waiter = next;
    }

    sem->waiters = NULL;
    sem->value = 0;

    spinlock_release(&sem->lock);
}
