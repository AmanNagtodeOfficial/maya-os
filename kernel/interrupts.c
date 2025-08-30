/**
 * Maya OS Interrupt Handler
 * Updated: 2025-08-29 10:54:58 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/interrupts.h"
#include "kernel/io.h"
#include "libc/stdio.h"

#define IDT_SIZE 256
#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[IDT_SIZE];
static idt_ptr_t idtp;
static isr_t interrupt_handlers[IDT_SIZE];
static bool interrupts_initialized = false;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

bool idt_install(void) {
    if (interrupts_initialized) {
        return true;
    }
    
    // Set up IDT pointer
    idtp.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idtp.base = (uint32_t)&idt;
    
    // Clear IDT and handlers
    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);
    memset(&interrupt_handlers, 0, sizeof(isr_t) * IDT_SIZE);
    
    // Remap PIC
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
    
    // Install ISR handlers
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    // ... Install other ISRs ...
    
    // Load IDT
    idt_load();
    
    interrupts_initialized = true;
    return true;
}

void isr_handler(struct regs *r) {
    if (interrupt_handlers[r->int_no]) {
        interrupt_handlers[r->int_no](r);
    } else {
        printf("Unhandled interrupt: %d\n", r->int_no);
    }
}

void irq_handler(struct regs *r) {
    // Send EOI to PICs
    if (r->int_no >= 40) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
    
    if (interrupt_handlers[r->int_no]) {
        interrupt_handlers[r->int_no](r);
    }
}

bool irq_install_handler(int irq, isr_t handler) {
    if (irq < 0 || irq >= IDT_SIZE || !handler) {
        return false;
    }
    
    interrupt_handlers[irq] = handler;
    return true;
}

void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < IDT_SIZE) {
        interrupt_handlers[irq] = NULL;
    }
}

bool interrupts_are_enabled(void) {
    uint32_t flags;
    __asm__ volatile ("pushf\n\t"
                      "pop %0"
                      : "=r"(flags));
    return flags & 0x200;
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void disable_interrupts(void) {
    __asm__ volatile ("cli");
}
