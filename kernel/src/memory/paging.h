#ifndef PAGING_H
#define PAGING_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem/mem.h>

#define ALIGN_TO_PAGE(x) (((x) + 4095) & ~4095)  // Align to the nearest 4096 bytes

#define POOL_SIZE (1ULL * 1024 * 1024 * 1024)
#define BLOCK_SIZE (4 * 1024)
#define NUM_BLOCKS (POOL_SIZE / BLOCK_SIZE)

extern uint8_t block_bitmap[NUM_BLOCKS / 8];

void pool_init();
void* page_alloc();
void* page_alloc_n(size_t pages);
void page_free(void* ptr);
void page_free_n(void* ptr, size_t pages);

void page_release();

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4
#define PAGE_SIZE    0x1000

#define HHDM         0xFFFFFFFF80000000
#define PHYS_TO_VIRT(p) ((void *)((uint64_t)(p) + HHDM))
#define VIRT_TO_PHYS(v) ((uint64_t)(v) - HHDM)

typedef uint64_t pt_entry_t;

typedef struct {
    pt_entry_t entries[512];
} page_table_t;

void InitPaging();
void mmap(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void umap(uint64_t vaddr);

#endif /* PAGING_H */