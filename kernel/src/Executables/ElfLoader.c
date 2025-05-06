#include "ElfLoader.h"
#include <elf.h>

void ExecElf(void* ptr) {
    Elf64_Ehdr* hdr = (Elf64_Ehdr*)ptr;

    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' || hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') return;
    if (hdr->e_ident[4] != 2) return;
    if (hdr->e_machine != 0x3E) return;
    if (hdr->e_type != 2) return;

    Elf64_Phdr* phdr = (Elf64_Phdr*)((uint8_t*)ptr + hdr->e_phoff);
    for (uint16_t i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type != 1) continue;
        uint8_t* src = (uint8_t*)ptr + phdr[i].p_offset;
        uint8_t* dst = (uint8_t*)phdr[i].p_vaddr;
        for (uint64_t j = 0; j < phdr[i].p_filesz; j++) {
            dst[j] = src[j];
        }
        for (uint64_t j = phdr[i].p_filesz; j < phdr[i].p_memsz; j++) {
            dst[j] = 0;
        }
    }

    void (*entry)() = (void (*)())hdr->e_entry;
    entry();
}