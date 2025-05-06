#include "paging.h"
#include <stdint.h>
#include <stddef.h>

static uint64_t next_page_table_addr = 0x100000;

static void *alloc_page_table() {
    void *addr = PHYS_TO_VIRT(next_page_table_addr);
    next_page_table_addr += PAGE_SIZE;
    for (size_t i = 0; i < PAGE_SIZE / sizeof(uint64_t); i++) ((uint64_t *)addr)[i] = 0;
    return addr;
}

static page_table_t *pml4;

void InitPaging() {
    pml4 = alloc_page_table();

    for (uint64_t i = 0; i < 0x100000000; i += PAGE_SIZE) {
        mmap(i, i, PAGE_PRESENT | PAGE_RW);
        mmap(HHDM + i, i, PAGE_PRESENT | PAGE_RW);
    }

    uint64_t cr3 = VIRT_TO_PHYS(pml4);
    asm volatile("mov %0, %%cr3" :: "r"(cr3));
}

void mmap(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint16_t pml4_i = (vaddr >> 39) & 0x1FF;
    uint16_t pdpt_i = (vaddr >> 30) & 0x1FF;
    uint16_t pd_i   = (vaddr >> 21) & 0x1FF;
    uint16_t pt_i   = (vaddr >> 12) & 0x1FF;

    page_table_t *pdpt, *pd, *pt;

    if (!(pml4->entries[pml4_i] & PAGE_PRESENT)) {
        pdpt = alloc_page_table();
        pml4->entries[pml4_i] = VIRT_TO_PHYS(pdpt) | flags;
    } else {
        pdpt = PHYS_TO_VIRT(pml4->entries[pml4_i] & ~0xFFF);
    }

    if (!(pdpt->entries[pdpt_i] & PAGE_PRESENT)) {
        pd = alloc_page_table();
        pdpt->entries[pdpt_i] = VIRT_TO_PHYS(pd) | flags;
    } else {
        pd = PHYS_TO_VIRT(pdpt->entries[pdpt_i] & ~0xFFF);
    }

    if (!(pd->entries[pd_i] & PAGE_PRESENT)) {
        pt = alloc_page_table();
        pd->entries[pd_i] = VIRT_TO_PHYS(pt) | flags;
    } else {
        pt = PHYS_TO_VIRT(pd->entries[pd_i] & ~0xFFF);
    }

    pt->entries[pt_i] = paddr | flags;
}

void umap(uint64_t vaddr) {
    uint16_t pml4_i = (vaddr >> 39) & 0x1FF;
    uint16_t pdpt_i = (vaddr >> 30) & 0x1FF;
    uint16_t pd_i   = (vaddr >> 21) & 0x1FF;
    uint16_t pt_i   = (vaddr >> 12) & 0x1FF;

    if (!(pml4->entries[pml4_i] & PAGE_PRESENT)) return;
    page_table_t *pdpt = PHYS_TO_VIRT(pml4->entries[pml4_i] & ~0xFFF);

    if (!(pdpt->entries[pdpt_i] & PAGE_PRESENT)) return;
    page_table_t *pd = PHYS_TO_VIRT(pdpt->entries[pdpt_i] & ~0xFFF);

    if (!(pd->entries[pd_i] & PAGE_PRESENT)) return;
    page_table_t *pt = PHYS_TO_VIRT(pd->entries[pd_i] & ~0xFFF);

    pt->entries[pt_i] = 0;
    asm volatile("invlpg (%0)" :: "r" (vaddr) : "memory");
}
