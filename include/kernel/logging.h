/**
 * Maya OS Kernel Logging System
 * Author: AmanNagtodeOfficial
 */
#ifndef KERNEL_LOGGING_H
#define KERNEL_LOGGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    KLOG_DEBUG = 0,
    KLOG_INFO,
    KLOG_WARN,
    KLOG_ERROR,
    KLOG_PANIC
} klog_level_t;

#define KLOG_RING_SIZE 4096

typedef struct {
    char     buf[KLOG_RING_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} klog_ring_t;

bool  klog_init(void);
void  klog(klog_level_t level, const char *fmt, ...);
void  klog_flush(void);
void  klog_dump_serial(void);
bool  klog_is_initialized(void);

/* Convenience macros */
#define KLOG_D(fmt, ...) klog(KLOG_DEBUG, fmt, ##__VA_ARGS__)
#define KLOG_I(fmt, ...) klog(KLOG_INFO,  fmt, ##__VA_ARGS__)
#define KLOG_W(fmt, ...) klog(KLOG_WARN,  fmt, ##__VA_ARGS__)
#define KLOG_E(fmt, ...) klog(KLOG_ERROR, fmt, ##__VA_ARGS__)
#define KLOG_P(fmt, ...) klog(KLOG_PANIC, fmt, ##__VA_ARGS__)

#endif /* KERNEL_LOGGING_H */
