#include "heap.h"
#include <mem/mem.h>
#include "paging.h"

static AllocationEntry allocation_table[MAX_ALLOCATIONS];  // Table to keep track of allocations

// Initialize the heap allocator (can be used to set up any additional structures if needed)
void heap_init() {
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        allocation_table[i].ptr = NULL;
        allocation_table[i].size = 0;
        allocation_table[i].in_use = false;
    }
}

// Malloc implementation (using the paging allocator)
void* malloc(size_t size) {
    // Align the size to the nearest page (4096 bytes)
    size_t aligned_size = ALIGN_TO_PAGE(size);
    size_t pages_needed = aligned_size / 4096;

    // Search the allocation table for a free block
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!allocation_table[i].in_use) {
            // Allocate pages
            void* ptr = page_alloc_n(pages_needed);
            if (ptr == NULL) {
                return NULL;  // Allocation failed
            }

            // Record the allocation in the table
            allocation_table[i].ptr = ptr;
            allocation_table[i].size = aligned_size;      // Set the size of allocation
            allocation_table[i].in_use = true;             // Mark as in use
            return ptr;
        }
    }
    return NULL;  // No available blocks
}

// Free implementation (using the paging allocator)
void free(void* ptr) {
    // Find the allocation in the table
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (allocation_table[i].in_use && allocation_table[i].ptr == ptr) {
            // Calculate the number of pages to free
            size_t pages_to_free = allocation_table[i].size / 4096;

            // Free the allocated pages
            page_free_n(ptr, pages_to_free);

            // Mark the allocation as free in the table
            allocation_table[i].in_use = false;
            allocation_table[i].ptr = NULL;
            allocation_table[i].size = 0;
            return;
        }
    }
}
