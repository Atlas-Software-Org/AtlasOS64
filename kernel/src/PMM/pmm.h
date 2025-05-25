#ifndef PMM_H
#define PMM_H 1

#include <stdint.h>
#include <stddef.h>

int KiPmmInit(uint64_t total_mem_bytes, uint32_t page_size, uint8_t *bitmap_mem, size_t bitmap_mem_size, size_t endKernelAligned_);
int64_t KiPmmAlloc();
void* KiPmmNAlloc(size_t count);
void KiPmmFree(size_t frame);
void KiPmmNFree(void* frame_ptr, size_t count);
size_t KiPmmGetTotalPages();
size_t KiPmmGetFreePages();

#endif /* PMM_H */
