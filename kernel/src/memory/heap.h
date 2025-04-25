#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_ALLOCATIONS 1024  // Arbitrary number, can be adjusted as needed

// Helper macro for aligning to page boundaries (4096 bytes)
#define ALIGN_TO_PAGE(x) (((x) + 4095) & ~4095)  // Align to the nearest 4096 bytes

// Structure to track each allocation
typedef struct {
    void* ptr;        // Base address of allocation
    size_t size;      // Size of the allocation (aligned to page size)
    bool in_use;      // Allocation status: true if in use, false if free
} AllocationEntry;

// Function prototypes
void heap_init();  // Initializes the heap allocator

// Wrapper functions for dynamic memory management
void* malloc(size_t size);  // Allocates a block of memory
void free(void* ptr);       // Frees the previously allocated memory

#endif // HEAP_H
