/**
 * Maya OS Crash Handler
 * Author: AmanNagtodeOfficial
 */
#ifndef KERNEL_CRASH_HANDLER_H
#define KERNEL_CRASH_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel/interrupts.h"

bool crash_handler_init(void);
void crash_dump(struct regs *r, const char *reason);
void crash_dump_stack(uint32_t esp, uint32_t depth);
void crash_emergency_save(void);

#endif /* KERNEL_CRASH_HANDLER_H */
