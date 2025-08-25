#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

// Memory map entry
struct memory_map_entry {
    uint32_t size;
        uint64_t address;
            uint64_t length;
                uint32_t type;
                } __attribute__((packed));

                // Physical memory manager
                void pmm_init(struct multiboot_info *mbi);
                void *pmm_alloc_page(void);
                void pmm_free_page(void *page);
                uint32_t pmm_get_total_memory(void);
                uint32_t pmm_get_used_memory(void);

                // Virtual memory manager
                void vmm_init(void);
                void *vmm_alloc(size_t size);
                void vmm_free(void *ptr);
                void vmm_map_page(uint32_t virtual, uint32_t physical, uint32_t flags);
                void vmm_unmap_page(uint32_t virtual);

                // Heap
                void heap_init(void);
                void *kmalloc(size_t size);
                void *kcalloc(size_t nmemb, size_t size);
                void *krealloc(void *ptr, size_t size);
                void kfree(void *ptr);

                #endif
                