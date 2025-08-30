/**
 * Maya OS Message Queue Implementation
 * Updated: 2025-08-29 11:18:54 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/message_queue.h"
#include "kernel/memory.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "libc/string.h"

#define MAX_MESSAGE_SIZE 1024
#define MAX_MESSAGES 64

typedef struct message {
    size_t size;
    uint8_t data[MAX_MESSAGE_SIZE];
    struct message* next;
} message_t;

typedef struct {
    message_t* first;
    message_t* last;
    size_t count;
    size_t max_messages;
    size_t max_size;
    mutex_t lock;
    condition_t not_full;
    condition_t not_empty;
    bool closed;
} message_queue_t;

message_queue_t* msgqueue_create(size_t max_messages, size_t max_size) {
    if (max_messages == 0 || max_size == 0 || 
        max_messages > MAX_MESSAGES || max_size > MAX_MESSAGE_SIZE) {
        return NULL;
    }

    message_queue_t* queue = kmalloc(sizeof(message_queue_t));
    if (!queue) {
        return NULL;
    }

    queue->first = NULL;
    queue->last = NULL;
    queue->count = 0;
    queue->max_messages = max_messages;
    queue->max_size = max_size;
    queue->closed = false;

    mutex_init(&queue->lock);
    condition_init(&queue->not_full);
    condition_init(&queue->not_empty);

    return queue;
}

bool msgqueue_send(message_queue_t* queue, const void* data, size_t size) {
    if (!queue || !data || size == 0 || size > queue->max_size || queue->closed) {
        return false;
    }

    mutex_lock(&queue->lock);

    // Wait while queue is full
    while (queue->count >= queue->max_messages && !queue->closed) {
        condition_wait(&queue->not_full, &queue->lock);
    }

    if (queue->closed) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Allocate new message
    message_t* msg = kmalloc(sizeof(message_t));
    if (!msg) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Copy message data
    msg->size = size;
    memcpy(msg->data, data, size);
    msg->next = NULL;

    // Add to queue
    if (!queue->first) {
        queue->first = msg;
        queue->last = msg;
    } else {
        queue->last->next = msg;
        queue->last = msg;
    }

    queue->count++;

    // Signal waiting receivers
    condition_signal(&queue->not_empty);

    mutex_unlock(&queue->lock);
    return true;
}

bool msgqueue_receive(message_queue_t* queue, void* buffer, size_t* size) {
    if (!queue || !buffer || !size || queue->closed) {
        return false;
    }

    mutex_lock(&queue->lock);

    // Wait while queue is empty
    while (queue->count == 0 && !queue->closed) {
        condition_wait(&queue->not_empty, &queue->lock);
    }

    if (queue->count == 0 && queue->closed) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Get first message
    message_t* msg = queue->first;
    queue->first = msg->next;
    if (!queue->first) {
        queue->last = NULL;
    }

    queue->count--;

    // Copy message data
    if (*size < msg->size) {
        *size = msg->size;
        kfree(msg);
        mutex_unlock(&queue->lock);
        return false;
    }

    memcpy(buffer, msg->data, msg->size);
    *size = msg->size;

    kfree(msg);

    // Signal waiting senders
    condition_signal(&queue->not_full);

    mutex_unlock(&queue->lock);
    return true;
}

bool msgqueue_try_send(message_queue_t* queue, const void* data, size_t size) {
    if (!queue || !data || size == 0 || size > queue->max_size || queue->closed) {
        return false;
    }

    mutex_lock(&queue->lock);

    if (queue->count >= queue->max_messages) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Allocate new message
    message_t* msg = kmalloc(sizeof(message_t));
    if (!msg) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Copy message data
    msg->size = size;
    memcpy(msg->data, data, size);
    msg->next = NULL;

    // Add to queue
    if (!queue->first) {
        queue->first = msg;
        queue->last = msg;
    } else {
        queue->last->next = msg;
        queue->last = msg;
    }

    queue->count++;

    // Signal waiting receivers
    condition_signal(&queue->not_empty);

    mutex_unlock(&queue->lock);
    return true;
}

bool msgqueue_try_receive(message_queue_t* queue, void* buffer, size_t* size) {
    if (!queue || !buffer || !size || queue->closed) {
        return false;
    }

    mutex_lock(&queue->lock);

    if (queue->count == 0) {
        mutex_unlock(&queue->lock);
        return false;
    }

    // Get first message
    message_t* msg = queue->first;
    queue->first = msg->next;
    if (!queue->first) {
        queue->last = NULL;
    }

    queue->count--;

    // Copy message data
    if (*size < msg->size) {
        *size = msg->size;
        kfree(msg);
        mutex_unlock(&queue->lock);
        return false;
    }

    memcpy(buffer, msg->data, msg->size);
    *size = msg->size;

    kfree(msg);

    // Signal waiting senders
    condition_signal(&queue->not_full);

    mutex_unlock(&queue->lock);
    return true;
}

void msgqueue_close(message_queue_t* queue) {
    if (!queue) {
        return;
    }

    mutex_lock(&queue->lock);
    queue->closed = true;
    mutex_unlock(&queue->lock);

    // Wake up all waiting processes
    condition_broadcast(&queue->not_full);
    condition_broadcast(&queue->not_empty);
}

void msgqueue_destroy(message_queue_t* queue) {
    if (!queue) {
        return;
    }

    msgqueue_close(queue);

    mutex_lock(&queue->lock);

    // Free all queued messages
    while (queue->first) {
        message_t* msg = queue->first;
        queue->first = msg->next;
        kfree(msg);
    }

    mutex_unlock(&queue->lock);

    // Clean up synchronization objects
    condition_destroy(&queue->not_full);
    condition_destroy(&queue->not_empty);

    kfree(queue);
}

size_t msgqueue_get_count(message_queue_t* queue) {
    if (!queue) {
        return 0;
    }

    mutex_lock(&queue->lock);
    size_t count = queue->count;
    mutex_unlock(&queue->lock);

    return count;
}

bool msgqueue_is_full(message_queue_t* queue) {
    if (!queue) {
        return true;
    }

    mutex_lock(&queue->lock);
    bool is_full = queue->count >= queue->max_messages;
    mutex_unlock(&queue->lock);

    return is_full;
}

bool msgqueue_is_empty(message_queue_t* queue) {
    if (!queue) {
        return true;
    }

    mutex_lock(&queue->lock);
    bool is_empty = queue->count == 0;
    mutex_unlock(&queue->lock);

    return is_empty;
}

bool msgqueue_is_closed(message_queue_t* queue) {
    if (!queue) {
        return true;
    }

    mutex_lock(&queue->lock);
    bool is_closed = queue->closed;
    mutex_unlock(&queue->lock);

    return is_closed;
}
