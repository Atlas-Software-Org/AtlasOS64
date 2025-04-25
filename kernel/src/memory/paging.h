#pragma once

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

typedef struct {
    int MappingId;
    uint64_t virtAddr;
    uint64_t flags;
    int mapped;
} MappingMetadata;

#define MMAP_INFO_ALREADY_MAPPED         1
#define MMAP_INFO_PARTIALLY_MAPPED       2
#define MMAP_INFO_ALIGNMENT_ADJUSTED     3
#define MMAP_INFO_BUFFERED_COPY          4

#define MMAP_SUCCESS                     0

#define MMAP_EPERM                      -1  // Operation not permitted
#define MMAP_ENOMEM                     -2  // Out of memory
#define MMAP_EACCES                     -3  // Permission denied
#define MMAP_EFAULT                     -4  // Bad address
#define MMAP_EOVERFLOW                  -5  // Offset + length overflow
#define MMAP_EINVAL                     -6  // Invalid arguments
#define MMAP_EMM                        -7  // Too many memory mappings

#define MMAP_EBUFFER_OVERFLOW          -20  // Internal buffer array full
#define MMAP_EPHYS_COPY_FAILED         -21  // memcpy from physical address failed
#define MMAP_EINVALID_ALIGNMENT        -22  // Address not aligned
#define MMAP_EUNSUPPORTED_FLAGS        -23  // Unsupported flags passed to mapper

#define MAP_FLAG_EXECUTABLE   (1 << 0)
#define MAP_FLAG_WRITABLE     (1 << 1)
#define MAP_FLAG_READABLE     (1 << 2)
#define MAP_FLAG_VOLATILE     (1 << 3) // do not backup when unmapping

int MapMemory(void* physAddr, void* virtAddr, uint64_t flags);
int RemapMemory(void* virtAddrOld, void* virtAddrNew);
int UnmapMemory(void* virtAddr);
int IsMapped(void* virtAddr);

void *get_physaddr(void *virtualaddr);