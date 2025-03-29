#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#define SIZEOF_AXF64HDR 43

// AXF header structure for 64-bit architecture
typedef struct {
    uint8_t AI_MAG[3];  // Magic Bytes (3 bytes)
    uint64_t AEntry;    // Entry point (8 bytes)
    uint8_t AArch;      // Architecture (1 byte)
    uint8_t AVersion;   // Version (1 byte)
    uint16_t ATrgtOS;   // Target OS (2 bytes)
    uint8_t AAbi;       // ABI (1 byte)
    uint64_t CodeSegSize; // Code segment size (8 bytes)
    uint64_t DataSegSize; // Data segment size (8 bytes)
    uint64_t BssSegSize;  // BSS segment size (8 bytes)
    uint8_t ABSS0;       // BSS segment initialization flag (1 byte)
    uint8_t EndHdrMag[2]; // End Header Magic (2 bytes)
} __attribute__((packed)) AHdr64_t;

// Helper function to read ELF file content into memory
void* read_file(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open ELF file");
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void *data = malloc(*size);
    if (!data) {
        perror("Failed to allocate memory for ELF file");
        fclose(file);
        return NULL;
    }
    
    fread(data, *size, 1, file);
    fclose(file);
    return data;
}

// Function to write the AXF header to the output file
void write_axf_header(FILE *out_file, uint64_t entry, uint64_t code_size, uint64_t data_size, uint64_t bss_size) {
    AHdr64_t header = {0};

    // Magic Bytes
    header.AI_MAG[0] = 'A';
    header.AI_MAG[1] = 'X';
    header.AI_MAG[2] = 'F';

    // Set the header fields
    header.AEntry = entry;
    header.AArch = 0x01;  // 64-bit architecture
    header.AVersion = 0x01; // Version 1
    header.ATrgtOS = 0xFFFF; // AtlasOS64 target OS
    header.AAbi = 0xFF; // SYSV ABI
    header.CodeSegSize = code_size;
    header.DataSegSize = data_size;
    header.BssSegSize = bss_size;
    header.ABSS0 = 0x01; // Initialize BSS to zero
    header.EndHdrMag[0] = 0xE0;
    header.EndHdrMag[1] = 0xFA;

    // Write the header to the output file
    fwrite(&header, sizeof(header), 1, out_file);
}

// Function to write AXF sections (code, data, BSS) to the output file
void write_axf_section(FILE *out_file, const uint8_t *data, size_t size) {
    fwrite(data, size, 1, out_file);
}

// Function to extract code, data, and BSS segments from an ELF file
void extract_elf_sections(const void *elf_data, size_t elf_size, uint8_t **code_segment, size_t *code_size,
                           uint8_t **data_segment, size_t *data_size, uint8_t **bss_segment, size_t *bss_size,
                           uint64_t *entry_point) {
    // Parse the ELF header
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;
    
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "Only 64-bit ELF files are supported.\n");
        exit(1);
    }

    *entry_point = ehdr->e_entry;

    // Loop through the program headers to find the segments
    Elf64_Phdr *phdr = (Elf64_Phdr *)((uint8_t *)elf_data + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type == PT_LOAD) {
            // Code Segment (Text section)
            if (phdr[i].p_flags & PF_X) {
                *code_segment = (uint8_t *)elf_data + phdr[i].p_offset;
                *code_size = phdr[i].p_filesz;
            }
            // Data Segment
            if (phdr[i].p_flags & PF_W) {
                *data_segment = (uint8_t *)elf_data + phdr[i].p_offset;
                *data_size = phdr[i].p_filesz;
            }
        }
        // Handle BSS as a loadable segment that doesn't have data in the ELF, but the size is there
        if (phdr[i].p_type == PT_LOAD && phdr[i].p_filesz == 0 && phdr[i].p_memsz > 0) {
            *bss_segment = NULL;  // No actual data in the BSS section
            *bss_size = phdr[i].p_memsz;
        }
    }
}

// Main function to convert ELF to AXF
void elf_to_axf(const char *input_filename, const char *output_filename) {
    // Step 1: Read ELF file
    size_t elf_size;
    void *elf_data = read_file(input_filename, &elf_size);
    if (!elf_data) return;

    // Step 2: Extract ELF sections (code, data, BSS, entry point)
    uint8_t *code_segment = NULL, *data_segment = NULL, *bss_segment = NULL;
    size_t code_size = 0, data_size = 0, bss_size = 0;
    uint64_t entry_point = 0;

    extract_elf_sections(elf_data, elf_size, &code_segment, &code_size, &data_segment, &data_size, &bss_segment, &bss_size, &entry_point);

    // Step 3: Create the AXF file
    FILE *out_file = fopen(output_filename, "wb");
    if (!out_file) {
        perror("Failed to open output AXF file");
        free(elf_data);
        return;
    }

    // Step 4: Write the AXF header
    write_axf_header(out_file, entry_point, code_size, data_size, bss_size);

    // Step 5: Write the code, data, and BSS sections
    if (code_segment) {
        write_axf_section(out_file, code_segment, code_size);
    }
    if (data_segment) {
        write_axf_section(out_file, data_segment, data_size);
    }
    if (bss_size > 0) {
        // Write BSS as empty space (since no data in BSS in ELF)
        uint8_t *empty_bss = calloc(bss_size, 1);
        write_axf_section(out_file, empty_bss, bss_size);
        free(empty_bss);
    }

    // Step 6: Close the output file and clean up
    fclose(out_file);
    free(elf_data);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.elf> <output.axf>\n", argv[0]);
        return 1;
    }

    elf_to_axf(argv[1], argv[2]);
    return 0;
}
