/**
 * Maya OS Kernel
 * Updated: 2025-08-29 10:52:34 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/kernel.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "kernel/timer.h"
#include "kernel/process.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "drivers/serial.h"
#include "drivers/ata.h"
#include "gui/graphics.h"
#include "gui/window.h"
#include "fs/fat32.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define KERNEL_VERSION "1.0.0"
#define BUILD_DATE "2025-08-29"
#define MIN_MEMORY_MB 64
#define MAX_MEMORY_GB 4

struct system_state {
    bool interrupts_enabled;
    bool mmu_enabled;
    uint32_t total_memory;
    uint32_t free_memory;
    uint32_t process_count;
    char last_error[256];
} system_state;

void kernel_main(uint32_t magic, struct multiboot_info *multiboot_info) {
    // Initialize system state
    memset(&system_state, 0, sizeof(system_state));
    
    // Store multiboot info
    mbi = multiboot_info;
    initial_esp = 0; // Will be set by assembly code
    
    // Initialize early console for debug output
    vga_init();
    vga_clear();
    serial_init(COM1);
    
    debug_print("Maya OS - Starting kernel initialization...\n");
    
    // Validate multiboot magic
    if (magic != 0x2BADB002) {
        kernel_panic("Invalid multiboot magic number");
    }
    
    // Print system information
    printf("Maya OS v%s - x86 Operating System\n", KERNEL_VERSION);
    printf("Build date: %s\n", BUILD_DATE);
    printf("Copyright (c) 2025 Maya OS Project\n\n");
    
    // Memory validation
    uint32_t total_mb = pmm_get_total_memory() / (1024 * 1024);
    if (total_mb < MIN_MEMORY_MB) {
        kernel_panic("Insufficient system memory");
    }
    if (total_mb > (MAX_MEMORY_GB * 1024)) {
        kernel_panic("Memory size exceeds maximum supported");
    }
    
    // Initialize core subsystems with error checking
    if (!gdt_install()) {
        kernel_panic("Failed to initialize GDT");
    }
    printf("GDT initialized.\n");
    
    if (!idt_install()) {
        kernel_panic("Failed to initialize IDT");
    }
    if (!pic_init()) {
        kernel_panic("Failed to initialize PIC");
    }
    printf("IDT and PIC initialized.\n");
    
    // Initialize memory management
    if (!pmm_init(multiboot_info)) {
        kernel_panic("Failed to initialize physical memory manager");
    }
    if (!vmm_init()) {
        kernel_panic("Failed to initialize virtual memory manager");
    }
    if (!heap_init()) {
        kernel_panic("Failed to initialize kernel heap");
    }
    printf("Memory management initialized.\n");
    
    // Initialize devices
    if (!timer_init(100)) {
        kernel_panic("Failed to initialize system timer");
    }
    if (!keyboard_init()) {
        kernel_panic("Failed to initialize keyboard");
    }
    if (!ata_init()) {
        kernel_panic("Failed to initialize ATA controller");
    }
    
    // Initialize filesystem
    if (!fat32_init(0)) {
        kernel_panic("Failed to initialize filesystem");
    }
    
    // Initialize process management
    if (!process_init()) {
        kernel_panic("Failed to initialize process manager");
    }
    
    // Initialize GUI system
    printf("Initializing GUI system...\n");
    if (!graphics_init()) {
        kernel_panic("Failed to initialize graphics system");
    }
    
    // Enable interrupts
    system_state.interrupts_enabled = true;
    sti();
    
    printf("\nMaya OS initialization complete!\n");
    printf("System ready. Type 'help' for available commands.\n\n");
    
    // Enter command loop
    command_loop();
}

void kernel_panic(const char *message) {
    // Disable interrupts
    cli();
    system_state.interrupts_enabled = false;
    
    // Save error message
    strncpy(system_state.last_error, message, sizeof(system_state.last_error) - 1);
    
    // Log system state
    debug_print("\n=== KERNEL PANIC ===\n");
    debug_print(message);
    debug_print("\nSystem State:\n");
    debug_print_state();
    
    // Attempt emergency data save
    emergency_data_save();
    
    // Visual panic message
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    printf("\n\nKERNEL PANIC: %s\n", message);
    printf("System halted.\n");
    
    // Halt system
    for (;;) {
        hlt();
    }
}

void debug_print_state(void) {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "Interrupts: %s\n"
        "MMU: %s\n"
        "Total Memory: %d MB\n"
        "Free Memory: %d MB\n"
        "Processes: %d\n"
        "Last Error: %s\n",
        system_state.interrupts_enabled ? "Enabled" : "Disabled",
        system_state.mmu_enabled ? "Enabled" : "Disabled",
        system_state.total_memory / (1024 * 1024),
        system_state.free_memory / (1024 * 1024),
        system_state.process_count,
        system_state.last_error
    );
    debug_print(buf);
}

void emergency_data_save(void) {
    // TODO: Implement emergency data saving
    debug_print("Emergency data save not implemented\n");
}
