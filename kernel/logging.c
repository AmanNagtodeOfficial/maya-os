/**
 * Maya OS Kernel Logging System
 * A ring-buffer based logger that outputs to serial port COM1.
 * Author: AmanNagtodeOfficial
 */

#include "kernel/logging.h"
#include "drivers/serial.h"
#include "libc/stdio.h"
#include "libc/string.h"

static klog_ring_t ring;
static bool        log_initialized = false;

static const char *level_str[] = {
    "[DEBUG] ",
    "[INFO]  ",
    "[WARN]  ",
    "[ERROR] ",
    "[PANIC] ",
};

/* Write a single character into the ring buffer */
static void ring_putc(char c) {
    ring.buf[ring.head] = c;
    ring.head = (ring.head + 1) % KLOG_RING_SIZE;
    if (ring.count < KLOG_RING_SIZE) {
        ring.count++;
    } else {
        /* Ring full – advance tail to overwrite oldest byte */
        ring.tail = (ring.tail + 1) % KLOG_RING_SIZE;
    }
}

/* Write a NUL-terminated string into the ring buffer and serial */
static void ring_puts(const char *s) {
    while (*s) {
        ring_putc(*s);
        serial_write_char(COM1, *s);
        s++;
    }
}

bool klog_init(void) {
    if (log_initialized) return true;

    memset(&ring, 0, sizeof(ring));
    log_initialized = true;
    klog(KLOG_INFO, "Kernel log subsystem initialized (ring=%u bytes)", KLOG_RING_SIZE);
    return true;
}

void klog(klog_level_t level, const char *fmt, ...) {
    if (!log_initialized) return;
    if (level < KLOG_DEBUG || level > KLOG_PANIC) return;

    char buf[256];

    /* Format message */
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Emit: level prefix + message + newline */
    ring_puts(level_str[level]);
    ring_puts(buf);
    ring_puts("\r\n");
}

void klog_flush(void) {
    /* In a fuller implementation this would drain the ring to disk.
       For now it is a no-op; data already went to serial in ring_puts. */
}

void klog_dump_serial(void) {
    if (!log_initialized) return;
    uint32_t idx   = ring.tail;
    uint32_t count = ring.count;
    while (count--) {
        serial_write_char(COM1, ring.buf[idx]);
        idx = (idx + 1) % KLOG_RING_SIZE;
    }
}

bool klog_is_initialized(void) {
    return log_initialized;
}
