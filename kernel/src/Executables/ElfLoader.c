#include "ElfLoader.h"
#include <elf.h>

int load_elf_binary(void* ptr, size_t size, uint64_t expected_load_address) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)ptr;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) return -1;
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) return -2;
    if (ehdr->e_type != ET_EXEC) return -3;
    if (ehdr->e_machine != EM_X86_64) return -4;

    Elf64_Phdr* phdr = (Elf64_Phdr*)((uint8_t*)ptr + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        void* dest = (void*)(expected_load_address + phdr[i].p_vaddr);
        void* src  = (uint8_t*)ptr + phdr[i].p_offset;

        memcpy(dest, src, phdr[i].p_filesz);

        if (phdr[i].p_memsz > phdr[i].p_filesz) {
            memset((uint8_t*)dest + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
        }
    }

    uint64_t entry_point = ehdr->e_entry + expected_load_address;

    void (*entry_func)() = (void (*)())entry_point;
    entry_func();

    return 0;
}