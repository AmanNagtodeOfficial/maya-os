/**
 * Maya OS Memory Management System
 * Updated: 2025-08-29 10:55:59 UTC
 * Author: AmanNagtodeOfficial
 */

#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "libc/string.h"

#define PAGE_SIZE 4096
#define HEAP_START 0xD0000000
#define HEAP_END   0xE0000000
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define MAX_MEMORY_BLOCKS 32768

typedef struct memory_block {
    uint32_t start;
    uint32_t size;
    bool used;
    struct memory_block* next;
} memory_block_t;

static uint32_t* page_directory = NULL;
static memory_block_t* heap_blocks = NULL;
static uint32_t total_memory = 0;
static uint32_t used_memory = 0;
static bool mmu_initialized = false;

// Memory map from multiboot info
static struct memory_map {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) *memory_map;

bool pmm_init(struct multiboot_info* mbi) {
    if (!mbi || !(mbi->flags & 0x40)) {
        return false;
    }

    memory_map = (struct memory_map*)mbi->mmap_addr;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    // Calculate total available memory
    total_memory = 0;
    while ((uint32_t)memory_map < mmap_end) {
        if (memory_map->type == 1) { // Available memory
            total_memory += memory_map->length;
        }
        memory_map = (struct memory_map*)((uint32_t)memory_map + memory_map->length + sizeof(uint32_t));
    }

    return true;
}

bool vmm_init(void) {
    if (mmu_initialized) {
        return true;
    }

    // Allocate page directory
    page_directory = (uint32_t*)kmalloc_aligned(PAGE_SIZE);
    if (!page_directory) {
        return false;
    }

    // Clear page directory
    memset(page_directory, 0, PAGE_SIZE);

    // Identity map first 4MB
    for (uint32_t i = 0; i < 1024; i++) {
        page_map(i * PAGE_SIZE, i * PAGE_SIZE);
    }

    // Set up page directory
    uint32_t cr0;
    __asm__ volatile(
        "mov %%cr0, %0"
        : "=r"(cr0)
    );
    cr0 |= 0x80000000; // Enable paging

    __asm__ volatile(
        "mov %0, %%cr3
"
        "mov %1, %%cr0"
        : : "r"(page_directory), "r"(cr0)
    );

    mmu_initialized = true;
    return true;
}

bool page_map(uint32_t virtual_addr, uint32_t physical_addr) {
    if (!mmu_initialized) {
        return false;
    }

    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    // Get/Create page table
    uint32_t* page_table;
    if (!(page_directory[pd_index] & 1)) {
        page_table = (uint32_t*)kmalloc_aligned(PAGE_SIZE);
        if (!page_table) {
            return false;
        }
        memset(page_table, 0, PAGE_SIZE);
        page_directory[pd_index] = ((uint32_t)page_table) | 3; // Present + RW
    } else {
        page_table = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    }

    // Map the page
    page_table[pt_index] = (physical_addr & 0xFFFFF000) | 3;
    
    // Invalidate TLB entry
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr));

    return true;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Round up to 4-byte alignment
    size = (size + 3) & ~3;

    memory_block_t* block = heap_blocks;
    memory_block_t* best_fit = NULL;
    uint32_t best_size = 0xFFFFFFFF;

    // Find best fit block
    while (block) {
        if (!block->used && block->size >= size) {
            if (block->size < best_size) {
                best_fit = block;
                best_size = block->size;
            }
        }
        block = block->next;
    }

    if (!best_fit) {
        return NULL;
    }

    // Split block if too large
    if (best_fit->size > size + sizeof(memory_block_t) + 4) {
        memory_block_t* new_block = (memory_block_t*)(best_fit->start + size);
        new_block->start = best_fit->start + size;
        new_block->size = best_fit->size - size - sizeof(memory_block_t);
        new_block->used = false;
        new_block->next = best_fit->next;
        
        best_fit->size = size;
        best_fit->next = new_block;
    }

    best_fit->used = true;
    used_memory += best_fit->size;

    return (void*)best_fit->start;
}

void* kmalloc_aligned(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Round up to page size
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    void* ptr = kmalloc(size + PAGE_SIZE);
    if (!ptr) {
        return NULL;
    }

    uint32_t addr = (uint32_t)ptr;
    uint32_t aligned_addr = (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    if (addr != aligned_addr) {
        kfree(ptr);
        ptr = kmalloc(size + (aligned_addr - addr));
        if (!ptr) {
            return NULL;
        }
    }

    return (void*)aligned_addr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    memory_block_t* block = heap_blocks;
    while (block) {
        if (block->start == (uint32_t)ptr) {
            block->used = false;
            used_memory -= block->size;

            // Merge with next block if free
            if (block->next && !block->next->used) {
                block->size += block->next->size + sizeof(memory_block_t);
                block->next = block->next->next;
            }

            // Find previous block
            memory_block_t* prev = heap_blocks;
            while (prev && prev->next != block) {
                prev = prev->next;
            }

            // Merge with previous block if free
            if (prev && !prev->used) {
                prev->size += block->size + sizeof(memory_block_t);
                prev->next = block->next;
            }

            break;
        }
        block = block->next;
    }
}

uint32_t get_total_memory(void) {
    return total_memory;
}

uint32_t get_used_memory(void) {
    return used_memory;
}

bool is_mmu_initialized(void) {
    return mmu_initialized;
}
