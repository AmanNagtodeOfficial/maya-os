/**
 * Maya OS Crash Handler
 * Registers fault ISRs and provides register/stack-trace dumps over serial.
 * Author: AmanNagtodeOfficial
 */

#include "kernel/crash_handler.h"
#include "kernel/logging.h"
#include "kernel/interrupts.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "libc/stdio.h"
#include "libc/string.h"

/* ─── helpers ──────────────────────────────────────────────────── */

static void dump_regs(struct regs *r) {
    char line[128];
    snprintf(line, sizeof(line),
        "EAX=%08x EBX=%08x ECX=%08x EDX=%08x\r\n"
        "ESI=%08x EDI=%08x EBP=%08x ESP=%08x\r\n"
        "EIP=%08x EFLAGS=%08x CS=%04x DS=%04x\r\n",
        r->eax, r->ebx, r->ecx, r->edx,
        r->esi, r->edi, r->ebp, r->esp,
        r->eip, r->eflags, r->cs, r->ds);
    KLOG_E("%s", line);
    serial_write_string(COM1, line);
}

/* ─── fault ISR callbacks ──────────────────────────────────────── */

static void fault_divide_by_zero(struct regs *r) {
    crash_dump(r, "Division by Zero (#DE)");
}

static void fault_general_protection(struct regs *r) {
    char msg[64];
    snprintf(msg, sizeof(msg), "General Protection Fault (error=%08x)", r->err_code);
    crash_dump(r, msg);
}

static void fault_page_fault(struct regs *r) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    char msg[64];
    snprintf(msg, sizeof(msg), "Page Fault at CR2=%08x (error=%08x)", cr2, r->err_code);
    crash_dump(r, msg);
}

static void fault_double_fault(struct regs *r) {
    crash_dump(r, "Double Fault (#DF) – system unstable");
}

/* ─── public API ───────────────────────────────────────────────── */

bool crash_handler_init(void) {
    interrupt_register_handler(0,  fault_divide_by_zero);
    interrupt_register_handler(8,  fault_double_fault);
    interrupt_register_handler(13, fault_general_protection);
    interrupt_register_handler(14, fault_page_fault);
    KLOG_I("Crash handler registered for faults 0/8/13/14");
    return true;
}

void crash_dump(struct regs *r, const char *reason) {
    /* Disable further interrupts */
    __asm__ volatile("cli");

    KLOG_P("*** CRASH: %s ***", reason ? reason : "Unknown");

    if (r) {
        dump_regs(r);
        crash_dump_stack(r->esp, 16);
    }

    /* Visible panic on screen */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printf("\n\n*** MAYA OS CRASH ***\n%s\nSystem halted.\n",
           reason ? reason : "Unknown reason");

    /* Attempt to save data before halting */
    crash_emergency_save();

    for (;;) __asm__ volatile("hlt");
}

void crash_dump_stack(uint32_t esp, uint32_t depth) {
    char line[64];
    snprintf(line, sizeof(line), "Stack trace (ESP=%08x):\r\n", esp);
    serial_write_string(COM1, line);

    uint32_t *frame = (uint32_t *)esp;
    for (uint32_t i = 0; i < depth; i++) {
        /* Bounds check – avoid walking off into unmapped memory */
        if ((uint32_t)(frame + i) >= 0xC0000000U) break;
        snprintf(line, sizeof(line), "  [%02u] %08x\r\n", i, frame[i]);
        serial_write_string(COM1, line);
    }
}

void crash_emergency_save(void) {
    /* Flush the kernel log ring to serial so the last messages are visible */
    klog_flush();
    klog_dump_serial();
}
