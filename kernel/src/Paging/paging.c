#include "paging.h"

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_RW        (1ULL << 1)
#define PAGE_PS        (1ULL << 7)

#define PAGE_ENTRIES 512
#define PAGE_SIZE_2MB (2 * 1024 * 1024)

// Adjust this for your HHDM base address
#define HHDM_BASE 0xFFFF800000000000ULL

typedef uint64_t pte_t;

typedef struct __attribute__((packed, aligned(4096))) {
    pte_t entries[PAGE_ENTRIES];
} page_table_t;

static page_table_t pml4;
static page_table_t pdpt_identity;
static page_table_t pd_identity;
static page_table_t pdpt_hhdm;
static page_table_t pd_hhdm;

pte_t* KiPml4Init() {
    // Zero all tables
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        pml4.entries[i] = 0;
        pdpt_identity.entries[i] = 0;
        pd_identity.entries[i] = 0;
        pdpt_hhdm.entries[i] = 0;
        pd_hhdm.entries[i] = 0;
    }

    // Map 512 entries in PD = 512 * 2MiB = 1GiB identity + HHDM mapping

    // Setup PD identity 2MB pages: physical 0 to 1GiB
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        uint64_t phys_addr = i * PAGE_SIZE_2MB;
        pd_identity.entries[i] = phys_addr | PAGE_PRESENT | PAGE_RW | PAGE_PS;
        pd_hhdm.entries[i] = phys_addr | PAGE_PRESENT | PAGE_RW | PAGE_PS;
    }

    // Setup PDPT identity and hhdm pointing to PDs
    pdpt_identity.entries[0] = ((uint64_t)&pd_identity) | PAGE_PRESENT | PAGE_RW;
    pdpt_hhdm.entries[0] = ((uint64_t)&pd_hhdm) | PAGE_PRESENT | PAGE_RW;

    // Setup PML4 entries:
    // Identity map in PML4 index 0
    pml4.entries[0] = ((uint64_t)&pdpt_identity) | PAGE_PRESENT | PAGE_RW;

    // HHDM map in PML4 index for HHDM_BASE
    size_t pml4_index_hhdm = (HHDM_BASE >> 39) & 0x1FF;
    pml4.entries[pml4_index_hhdm] = ((uint64_t)&pdpt_hhdm) | PAGE_PRESENT | PAGE_RW;

    printk("{ LOG }\tPHYS CR3: %lX / VIRT CR3: %lX\n\r", *(pte_t*)&pml4, *(uint64_t*)&pml4-0xFFFF800000000000);

    return (pte_t*)&pml4;
}