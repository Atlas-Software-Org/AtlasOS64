#pragma once

#include <stdint.h>
#include <stddef.h>
#include <mem/mem.h>

// Error Codes for load_elf_binary function
#define ELF_ERR_MAGIC     -1  // ELF magic number check failed
#define ELF_ERR_CLASS     -2  // Invalid ELF class (not 64-bit)
#define ELF_ERR_TYPE      -3  // Not an executable ELF file
#define ELF_ERR_MACHINE   -4  // Unsupported machine type (not x86_64)
#define ELF_ERR_PHDR      -5  // Program header table invalid or missing
#define ELF_ERR_LOAD_SEG  -6  // Failed to load segment (invalid segment type)
#define ELF_ERR_MEM_ALLOC -7  // Memory allocation failed (if applicable)
#define ELF_ERR_ENTRY     -8  // Entry point not valid

int load_elf_binary(void* ptr, size_t size, uint64_t expected_load_address);