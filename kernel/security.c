/**
 * Maya OS Security Model
 * Capability-based per-process permission table.
 * Author: AmanNagtodeOfficial
 */

#include "kernel/security.h"
#include "kernel/logging.h"
#include "kernel/process.h"
#include "libc/string.h"

#define MAX_SEC_ENTRIES 256

typedef struct {
    uint32_t pid;
    uint32_t caps;
    bool     active;
} sec_entry_t;

static sec_entry_t cap_table[MAX_SEC_ENTRIES];
static bool        sec_initialized = false;

bool sec_init(void) {
    if (sec_initialized) return true;
    memset(cap_table, 0, sizeof(cap_table));
    sec_initialized = true;
    KLOG_I("Security subsystem initialized (%u capability slots)", MAX_SEC_ENTRIES);
    return true;
}

/* Find an existing entry for this pid, or -1 */
static int sec_find_entry(uint32_t pid) {
    for (int i = 0; i < MAX_SEC_ENTRIES; i++) {
        if (cap_table[i].active && cap_table[i].pid == pid) return i;
    }
    return -1;
}

/* Allocate a new entry */
static int sec_alloc_entry(uint32_t pid) {
    for (int i = 0; i < MAX_SEC_ENTRIES; i++) {
        if (!cap_table[i].active) {
            cap_table[i].active = true;
            cap_table[i].pid    = pid;
            cap_table[i].caps   = 0;
            return i;
        }
    }
    return -1;
}

bool sec_grant_cap(uint32_t pid, uint32_t caps) {
    if (!sec_initialized) return false;
    int idx = sec_find_entry(pid);
    if (idx < 0) idx = sec_alloc_entry(pid);
    if (idx < 0) return false;
    cap_table[idx].caps |= caps;
    KLOG_D("PID %u granted caps 0x%08x", pid, caps);
    return true;
}

bool sec_check_cap(uint32_t pid, uint32_t cap) {
    if (!sec_initialized) return false;
    int idx = sec_find_entry(pid);
    if (idx < 0) return false;
    return (cap_table[idx].caps & cap) == cap;
}

bool sec_revoke_cap(uint32_t pid, uint32_t caps) {
    if (!sec_initialized) return false;
    int idx = sec_find_entry(pid);
    if (idx < 0) return false;
    cap_table[idx].caps &= ~caps;
    KLOG_D("PID %u revoked caps 0x%08x", pid, caps);
    return true;
}

uint32_t sec_get_caps(uint32_t pid) {
    if (!sec_initialized) return 0;
    int idx = sec_find_entry(pid);
    if (idx < 0) return 0;
    return cap_table[idx].caps;
}

bool sec_is_initialized(void) {
    return sec_initialized;
}
