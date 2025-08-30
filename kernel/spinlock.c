/**
 * Maya OS Spinlock Implementation
 * Updated: 2025-08-29 11:16:18 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/spinlock.h"
#include "kernel/interrupts.h"

void spinlock_init(spinlock_t* lock) {
    if (!lock) {
        return;
    }
    lock->locked = 0;
    lock->cpu = -1;
    lock->interrupt_flags = 0;
}

void spinlock_acquire(spinlock_t* lock) {
    if (!lock) {
        return;
    }

    // Save interrupt state and disable interrupts
    lock->interrupt_flags = interrupt_disable();

    // Get current CPU ID
    int cpu = apic_get_id();

    // Check for recursive locking
    if (lock->locked && lock->cpu == cpu) {
        // TODO: Handle deadlock detection
        interrupt_restore(lock->interrupt_flags);
        return;
    }

    // Attempt to acquire lock
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        // Spin until lock is available
        while (lock->locked) {
            __asm__ volatile("pause");
        }
    }

    // Memory barrier
    __sync_synchronize();

    // Set owner
    lock->cpu = cpu;
}

bool spinlock_try_acquire(spinlock_t* lock) {
    if (!lock) {
        return false;
    }

    // Save interrupt state and disable interrupts
    uint32_t flags = interrupt_disable();

    // Get current CPU ID
    int cpu = apic_get_id();

    // Check for recursive locking
    if (lock->locked && lock->cpu == cpu) {
        interrupt_restore(flags);
        return false;
    }

    // Attempt to acquire lock
    if (__sync_lock_test_and_set(&lock->locked, 1)) {
        interrupt_restore(flags);
        return false;
    }

    // Memory barrier
    __sync_synchronize();

    // Set owner and save interrupt flags
    lock->cpu = cpu;
    lock->interrupt_flags = flags;
    return true;
}

void spinlock_release(spinlock_t* lock) {
    if (!lock) {
        return;
    }

    // Verify owner
    int cpu = apic_get_id();
    if (lock->cpu != cpu) {
        return;
    }

    // Clear owner
    lock->cpu = -1;

    // Memory barrier
    __sync_synchronize();

    // Release lock
    lock->locked = 0;

    // Restore interrupt state
    interrupt_restore(lock->interrupt_flags);
}

bool spinlock_is_locked(spinlock_t* lock) {
    if (!lock) {
        return false;
    }
    return lock->locked != 0;
}

int spinlock_get_cpu(spinlock_t* lock) {
    if (!lock) {
        return -1;
    }
    return lock->cpu;
}
