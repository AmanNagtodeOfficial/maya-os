#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @file memory.h
 * @brief Enhanced kernel memory management with error handling and bounds checking
 * @author Maya OS Development Team
 * @date 2026-01-11
 */

/* Error codes for memory operations */
typedef enum {
    MEMORY_SUCCESS = 0,
    MEMORY_ERROR_INVALID_POINTER = -1,
    MEMORY_ERROR_OUT_OF_BOUNDS = -2,
    MEMORY_ERROR_ALLOCATION_FAILED = -3,
    MEMORY_ERROR_INSUFFICIENT_MEMORY = -4,
    MEMORY_ERROR_ALIGNMENT_FAILED = -5,
    MEMORY_ERROR_NULL_POINTER = -6,
    MEMORY_ERROR_DOUBLE_FREE = -7,
    MEMORY_ERROR_CORRUPTED_HEAP = -8,
    MEMORY_ERROR_INVALID_SIZE = -9,
} memory_error_t;

/* Memory region flags */
typedef enum {
    MEMORY_FLAG_READ = 1 << 0,
    MEMORY_FLAG_WRITE = 1 << 1,
    MEMORY_FLAG_EXECUTE = 1 << 2,
    MEMORY_FLAG_USER = 1 << 3,
    MEMORY_FLAG_KERNEL = 1 << 4,
    MEMORY_FLAG_CACHED = 1 << 5,
} memory_flags_t;

/* Memory allocation tracking structure */
typedef struct {
    void *ptr;
    size_t size;
    uint32_t magic;
    bool is_free;
    size_t allocation_id;
} memory_block_t;

/* Memory statistics structure */
typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    size_t allocation_count;
    size_t free_count;
    uint32_t fragmentation_ratio;
} memory_stats_t;

/* Physical memory range structure */
typedef struct {
    uintptr_t start;
    uintptr_t end;
    memory_flags_t flags;
    size_t size;
} memory_range_t;

/**
 * Initialize the kernel memory management system
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_init(void);

/**
 * Allocate memory with bounds checking
 * @param size Size of memory to allocate (must be > 0)
 * @return Pointer to allocated memory, NULL on failure
 */
void *memory_alloc(size_t size);

/**
 * Allocate aligned memory
 * @param size Size of memory to allocate
 * @param alignment Alignment requirement (must be power of 2)
 * @return Pointer to aligned memory, NULL on failure
 */
void *memory_alloc_aligned(size_t size, size_t alignment);

/**
 * Reallocate memory with bounds checking
 * @param ptr Existing memory pointer (can be NULL)
 * @param size New size (must be > 0)
 * @return Pointer to reallocated memory, NULL on failure
 */
void *memory_realloc(void *ptr, size_t size);

/**
 * Free allocated memory with validation
 * @param ptr Pointer to free (NULL is safe)
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_free(void *ptr);

/**
 * Secure memory free with zeroing
 * @param ptr Pointer to free
 * @param size Size of memory to zero before freeing
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_free_secure(void *ptr, size_t size);

/**
 * Validate pointer bounds and alignment
 * @param ptr Pointer to validate
 * @param size Size to check within bounds
 * @return true if valid, false otherwise
 */
bool memory_is_valid_pointer(const void *ptr, size_t size);

/**
 * Check if pointer is in bounds of allocated memory
 * @param ptr Pointer to check
 * @param size Size to verify
 * @return true if in bounds, false otherwise
 */
bool memory_is_in_bounds(const void *ptr, size_t size);

/**
 * Get size of allocated memory block
 * @param ptr Pointer to allocated memory
 * @return Size of block, 0 if invalid pointer
 */
size_t memory_get_block_size(const void *ptr);

/**
 * Copy memory with bounds checking
 * @param dst Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to copy
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_copy(void *dst, const void *src, size_t size);

/**
 * Move memory with overlap handling
 * @param dst Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to move
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_move(void *dst, const void *src, size_t size);

/**
 * Set memory with bounds checking
 * @param ptr Memory pointer
 * @param value Byte value to set
 * @param size Number of bytes to set
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_set(void *ptr, int value, size_t size);

/**
 * Compare memory with bounds checking
 * @param ptr1 First memory pointer
 * @param ptr2 Second memory pointer
 * @param size Number of bytes to compare
 * @return 0 if equal, non-zero otherwise
 */
int memory_compare(const void *ptr1, const void *ptr2, size_t size);

/**
 * Get current memory statistics
 * @param stats Pointer to memory_stats_t structure to fill
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_get_stats(memory_stats_t *stats);

/**
 * Enable memory debugging features
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_enable_debugging(void);

/**
 * Disable memory debugging features
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_disable_debugging(void);

/**
 * Dump memory allocation information for debugging
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_dump_allocations(void);

/**
 * Verify heap integrity
 * @return MEMORY_SUCCESS if heap is valid, error code otherwise
 */
memory_error_t memory_verify_heap(void);

/**
 * Map physical memory region with flags
 * @param phys_addr Physical address to map
 * @param virt_addr Virtual address to map to
 * @param size Size of region
 * @param flags Memory flags
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_map_region(uintptr_t phys_addr, uintptr_t virt_addr, 
                                  size_t size, memory_flags_t flags);

/**
 * Unmap physical memory region
 * @param virt_addr Virtual address to unmap
 * @param size Size of region
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_unmap_region(uintptr_t virt_addr, size_t size);

/**
 * Get memory range information
 * @param virt_addr Virtual address to query
 * @param range Pointer to memory_range_t to fill
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_get_range(uintptr_t virt_addr, memory_range_t *range);

/**
 * Lock memory region in physical memory (prevent swapping)
 * @param ptr Pointer to memory region
 * @param size Size of region
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_lock(void *ptr, size_t size);

/**
 * Unlock memory region (allow swapping)
 * @param ptr Pointer to memory region
 * @param size Size of region
 * @return MEMORY_SUCCESS on success, error code otherwise
 */
memory_error_t memory_unlock(void *ptr, size_t size);

/**
 * Get error message for error code
 * @param error Error code
 * @return Human-readable error message
 */
const char *memory_strerror(memory_error_t error);

#endif /* KERNEL_MEMORY_H */
