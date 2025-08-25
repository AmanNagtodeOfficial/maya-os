#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// IDT entry structure
struct idt_entry {
    uint16_t base_lo;
        uint16_t sel;
            uint8_t always0;
                uint8_t flags;
                    uint16_t base_hi;
                    } __attribute__((packed));

                    // IDT pointer structure
                    struct idt_ptr {
                        uint16_t limit;
                            uint32_t base;
                            } __attribute__((packed));

                            // Register structure for interrupt handlers
                            struct registers {
                                uint32_t gs, fs, es, ds;
                                    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
                                        uint32_t int_no, err_code;
                                            uint32_t eip, cs, eflags, useresp, ss;
                                            } __attribute__((packed));

                                            // Function prototypes
                                            void idt_init(void);
                                            void idt_install(void);
                                            void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

                                            void isr_handler(struct registers *regs);
                                            void irq_handler(struct registers *regs);

                                            typedef void (*isr_t)(struct registers *);
                                            void register_interrupt_handler(uint8_t n, isr_t handler);

                                            // PIC functions
                                            void pic_init(void);
                                            void pic_send_eoi(uint8_t irq);

                                            #endif
                                            