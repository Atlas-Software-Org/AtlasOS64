#pragma once

#include <stdint.h>
#include <stddef.h>

#define POOL_SIZE (1ULL * 1024 * 1024 * 1024)
#define BLOCK_SIZE (4 * 1024)
#define NUM_BLOCKS (POOL_SIZE / BLOCK_SIZE)

extern uint8_t block_bitmap[NUM_BLOCKS / 8];

void pool_init();
void* page_alloc();
void* page_alloc_n(size_t pages);
void page_free(void* ptr);
void page_free_n(void* ptr, size_t pages);
