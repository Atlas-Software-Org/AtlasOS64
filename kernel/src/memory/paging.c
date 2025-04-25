#include "paging.h"
#include <mem/mem.h>

__attribute__((aligned(4096))) static uint8_t memory_pool[POOL_SIZE];

uint8_t block_bitmap[NUM_BLOCKS / 8];

void pool_init() {
    memset(memory_pool, 0, sizeof(memory_pool));
    memset(block_bitmap, 0, sizeof(block_bitmap));
}

void* page_alloc() {
    for (uint64_t i = 0; i < NUM_BLOCKS; i++) {
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        if ((block_bitmap[byte_index] & (1 << bit_index)) == 0) {
            block_bitmap[byte_index] |= (1 << bit_index);
            return (void*)(memory_pool + (i * BLOCK_SIZE));
        }
    }
    return NULL;
}

void* page_alloc_n(size_t pages) {
    size_t consecutive = 0;
    for (uint64_t i = 0; i < NUM_BLOCKS; i++) {
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;

        if ((block_bitmap[byte_index] & (1 << bit_index)) == 0) {
            if (++consecutive == pages) {
                for (size_t j = i - pages + 1; j <= i; j++) {
                    byte_index = j / 8;
                    bit_index = j % 8;
                    block_bitmap[byte_index] |= (1 << bit_index);
                }
                return (void*)(memory_pool + ((i - pages + 1) * BLOCK_SIZE));
            }
        } else {
            consecutive = 0;
        }
    }
    return NULL;
}

void page_free(void* ptr) {
    uint64_t block_index = ((uint8_t*)ptr - memory_pool) / BLOCK_SIZE;
    uint64_t byte_index = block_index / 8;
    uint64_t bit_index = block_index % 8;

    block_bitmap[byte_index] &= ~(1 << bit_index);
}

void page_free_n(void* ptr, size_t pages) {
    uint64_t block_index = ((uint8_t*)ptr - memory_pool) / BLOCK_SIZE;
    for (size_t i = 0; i < pages; i++) {
        uint64_t byte_index = (block_index + i) / 8;
        uint64_t bit_index = (block_index + i) % 8;

        block_bitmap[byte_index] &= ~(1 << bit_index);
    }
}

void page_release() {
    for (int i = 0; i < (POOL_SIZE / 4096); i++) {
        page_free(&memory_pool[i * 4096]);
    }   
}
