/**
 * Maya OS Pipe Implementation
 * Updated: 2025-08-29 11:18:08 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/pipe.h"
#include "kernel/memory.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "libc/string.h"

#define PIPE_BUFFER_SIZE 4096

typedef struct {
    uint8_t* buffer;
    size_t read_pos;
    size_t write_pos;
    size_t count;
    semaphore_t read_sem;
    semaphore_t write_sem;
    mutex_t lock;
    bool closed;
} pipe_t;

bool pipe_create(int* read_fd, int* write_fd) {
    if (!read_fd || !write_fd) {
        return false;
    }

    // Allocate pipe structure
    pipe_t* pipe = kmalloc(sizeof(pipe_t));
    if (!pipe) {
        return false;
    }

    // Allocate buffer
    pipe->buffer = kmalloc(PIPE_BUFFER_SIZE);
    if (!pipe->buffer) {
        kfree(pipe);
        return false;
    }

    // Initialize pipe
    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->count = 0;
    pipe->closed = false;

    semaphore_init(&pipe->read_sem, 0);
    semaphore_init(&pipe->write_sem, PIPE_BUFFER_SIZE);
    mutex_init(&pipe->lock);

    // Create file descriptors
    *read_fd = process_alloc_fd(pipe, FD_TYPE_PIPE_READ);
    if (*read_fd < 0) {
        kfree(pipe->buffer);
        kfree(pipe);
        return false;
    }

    *write_fd = process_alloc_fd(pipe, FD_TYPE_PIPE_WRITE);
    if (*write_fd < 0) {
        process_free_fd(*read_fd);
        kfree(pipe->buffer);
        kfree(pipe);
        return false;
    }

    return true;
}

ssize_t pipe_read(pipe_t* pipe, void* buffer, size_t size) {
    if (!pipe || !buffer || size == 0 || pipe->closed) {
        return -1;
    }

    size_t bytes_read = 0;
    uint8_t* buf = (uint8_t*)buffer;

    while (bytes_read < size) {
        // Wait for data
        semaphore_wait(&pipe->read_sem);

        if (pipe->closed) {
            return bytes_read > 0 ? bytes_read : -1;
        }

        // Read data
        mutex_lock(&pipe->lock);

        size_t available = pipe->count;
        size_t to_read = size - bytes_read;
        if (to_read > available) {
            to_read = available;
        }

        for (size_t i = 0; i < to_read; i++) {
            buf[bytes_read++] = pipe->buffer[pipe->read_pos];
            pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
        }

        mutex_unlock(&pipe->lock);

        // Signal writer
        semaphore_signal(&pipe->write_sem);

        if (bytes_read == size || pipe->closed) {
            break;
        }
    }

    return bytes_read;
}

ssize_t pipe_write(pipe_t* pipe, const void* buffer, size_t size) {
    if (!pipe || !buffer || size == 0 || pipe->closed) {
        return -1;
    }

    size_t bytes_written = 0;
    const uint8_t* buf = (const uint8_t*)buffer;

    while (bytes_written < size) {
        // Wait for space
        semaphore_wait(&pipe->write_sem);

        if (pipe->closed) {
            return -1;
        }

        // Write data
        mutex_lock(&pipe->lock);

        size_t available = PIPE_BUFFER_SIZE - pipe->count;
        size_t to_write = size - bytes_written;
        if (to_write > available) {
            to_write = available;
        }

        for (size_t i = 0; i < to_write; i++) {
            pipe->buffer[pipe->write_pos] = buf[bytes_written++];
            pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
        }

        mutex_unlock(&pipe->lock);

        // Signal reader
        semaphore_signal(&pipe->read_sem);

        if (bytes_written == size || pipe->closed) {
            break;
        }
    }

    return bytes_written;
}

void pipe_close(pipe_t* pipe) {
    if (!pipe) {
        return;
    }

    mutex_lock(&pipe->lock);
    pipe->closed = true;
    mutex_unlock(&pipe->lock);

    // Wake up waiting processes
    semaphore_signal(&pipe->read_sem);
    semaphore_signal(&pipe->write_sem);
}

void pipe_destroy(pipe_t* pipe) {
    if (!pipe) {
        return;
    }

    pipe_close(pipe);

    // Clean up resources
    semaphore_destroy(&pipe->read_sem);
    semaphore_destroy(&pipe->write_sem);
    
    if (pipe->buffer) {
        kfree(pipe->buffer);
    }
    kfree(pipe);
}

bool pipe_is_closed(pipe_t* pipe) {
    if (!pipe) {
        return true;
    }
    return pipe->closed;
}

size_t pipe_get_available(pipe_t* pipe) {
    if (!pipe) {
        return 0;
    }

    mutex_lock(&pipe->lock);
    size_t count = pipe->count;
    mutex_unlock(&pipe->lock);

    return count;
}
