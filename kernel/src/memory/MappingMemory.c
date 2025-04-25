#include "paging.h"

#define MAX_NUM_OF_MAPPINGS 4096

int MappingIdx = 0;

uint8_t MemoryMappingBuffer[MAX_NUM_OF_MAPPINGS][4096]; // 16 MiB
MappingMetadata mappingMetadata[MAX_NUM_OF_MAPPINGS];

int MapMemory(void* physAddr, void* virtAddr, uint64_t flags __attribute__((unused))) {
    if (((uintptr_t)physAddr % 4096) != 0 || ((uintptr_t)virtAddr % 4096) != 0) {
        return MMAP_EINVALID_ALIGNMENT;
    }

    int slot = -1;
    for (int i = 0; i < MAX_NUM_OF_MAPPINGS; i++) {
        if (!mappingMetadata[i].mapped) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return MMAP_EBUFFER_OVERFLOW;
    }

    if (!(flags & MAP_FLAG_VOLATILE)) {
        if (memcpy(MemoryMappingBuffer[slot], virtAddr, 4096) == NULL) {
            return MMAP_EPHYS_COPY_FAILED;
        }
    }

    if (memcpy(virtAddr, physAddr, 4096) == NULL) {
        return MMAP_EPHYS_COPY_FAILED;
    }

    mappingMetadata[slot].MappingId = slot;
    mappingMetadata[slot].virtAddr = (uint64_t)virtAddr;
    mappingMetadata[slot].flags = flags;
    mappingMetadata[slot].mapped = 1;

    return MMAP_SUCCESS;
}

int RemapMemory(void* virtAddrOld, void* virtAddrNew) {
    for (int i = 0; i < MAX_NUM_OF_MAPPINGS; i++) {
        if (mappingMetadata[i].mapped && mappingMetadata[i].virtAddr == (uint64_t)virtAddrOld) {
            memcpy((void*)virtAddrNew, MemoryMappingBuffer[i], 4096);
            mappingMetadata[i].virtAddr = (uint64_t)virtAddrNew;
            return MMAP_SUCCESS;
        }
    }
    return MMAP_EFAULT;
}

int UnmapMemory(void* virtAddr) {
    for (int i = 0; i < MAX_NUM_OF_MAPPINGS; i++) {
        if (mappingMetadata[i].mapped && mappingMetadata[i].virtAddr == (uint64_t)virtAddr) {
            memcpy((void*)virtAddr, MemoryMappingBuffer[i], 4096);

            mappingMetadata[i].virtAddr = 0;
            mappingMetadata[i].mapped = 0;
            mappingMetadata[i].flags = 0;
            return MMAP_SUCCESS;
        }
    }

    return -1; // Could define MMAP_ENOMAP or similar
}

int IsMapped(void* virtAddr) {
    for (int i = 0; i < MAX_NUM_OF_MAPPINGS; i++) {
        if (mappingMetadata[i].mapped && mappingMetadata[i].virtAddr == (uint64_t)virtAddr) {
            return mappingMetadata[i].mapped ? 1 : 0; // Just to make sure
        }
    }

    return -1; // Non-existant entry
}

extern void outb(uint16_t, uint8_t);

void *get_physaddr(void *virtualaddr) {
    uint64_t va = (uint64_t)virtualaddr;

    uint64_t pml5_index = (va >> 48) & 0x1FF;
    uint64_t pml4_index = (va >> 39) & 0x1FF;
    uint64_t pdpt_index = (va >> 30) & 0x1FF;
    uint64_t pd_index   = (va >> 21) & 0x1FF;
    uint64_t pt_index   = (va >> 12) & 0x1FF;

    uint64_t *pml5 = (uint64_t *)(0xFFFFFFFFFFFFF000UL);
    if (!(pml5[pml5_index] & 1)) return 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint64_t *pml4 = (uint64_t *)(0xFFFFFFFFFF800000UL + (pml5_index << 12));
    outb(0xE9, 't');
    if (!(pml4[pml4_index] & 1)) return 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint64_t *pdpt = (uint64_t *)(0xFFFFFF8000000000UL + (pml5_index << 30) + (pml4_index << 21));
    outb(0xE9, 't');
    if (!(pdpt[pdpt_index] & 1)) return 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint64_t *pd = (uint64_t *)(0xFFFF800000000000UL + (pml5_index << 39) + (pml4_index << 30) + (pdpt_index << 21));
    outb(0xE9, 't');
    if (!(pd[pd_index] & 1)) return 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint64_t *pt = (uint64_t *)(0xFF00000000000000UL + (pml5_index << 48) + (pml4_index << 39) + (pdpt_index << 30) + (pd_index << 21));
    outb(0xE9, 't');
    if (!(pt[pt_index] & 1)) return 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint64_t phys = (pt[pt_index] & ~0xFFFUL) + (va & 0xFFF);
    outb(0xE9, 't');
    return (void *)phys;
    outb(0xE9, 't');
}