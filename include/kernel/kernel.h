#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Multiboot structures
struct multiboot_info {
    uint32_t flags;
        uint32_t mem_lower;
            uint32_t mem_upper;
                uint32_t boot_device;
                    uint32_t cmdline;
                        uint32_t mods_count;
                            uint32_t mods_addr;
                                uint32_t syms[4];
                                    uint32_t mmap_length;
                                        uint32_t mmap_addr;
                                            uint32_t drives_length;
                                                uint32_t drives_addr;
                                                    uint32_t config_table;
                                                        uint32_t boot_loader_name;
                                                            uint32_t apm_table;
                                                                uint32_t vbe_control_info;
                                                                    uint32_t vbe_mode_info;
                                                                        uint32_t vbe_mode;
                                                                            uint32_t vbe_interface_seg;
                                                                                uint32_t vbe_interface_off;
                                                                                    uint32_t vbe_interface_len;
                                                                                    } __attribute__((packed));

                                                                                    // System information
                                                                                    extern struct multiboot_info *mbi;
                                                                                    extern uint32_t initial_esp;

                                                                                    // Kernel functions
                                                                                    void kernel_main(uint32_t magic, struct multiboot_info *mbi);
                                                                                    void kernel_panic(const char *message);
                                                                                    void outb(uint16_t port, uint8_t value);
                                                                                    uint8_t inb(uint16_t port);
                                                                                    void outw(uint16_t port, uint16_t value);
                                                                                    uint16_t inw(uint16_t port);

                                                                                    // Debug functions
                                                                                    void debug_print(const char *message);
                                                                                    void debug_print_hex(uint32_t value);

                                                                                    #endif
                                                                                    