#ifndef APXF_H
#define APXF_H 1

#include <stdint.h>
#include <mem/mem.h>

typedef struct {
    uint8_t magic[4];       // {'\xAB', 'P', 'X', 'F'}
    uint16_t version;       // 0x0001
    uint16_t bitness;       // 0x0040
    uint32_t vendor_id;
    uint32_t code_offset;
    uint32_t code_size;
    uint32_t entry_point;
    uint32_t reserved;
    uint32_t checksum;      // Unused in this version
} __attribute__((packed)) ApxfHeader;

typedef struct {
    uint8_t ApxfStack[4096];
    uint64_t SBP;
    uint64_t SP;
    uint64_t Registers[4];
    uint64_t PopRegister;
    uint64_t IEP;
    uint64_t CallFrame[4096];
} ApxfExecutionFrame;

int ExecApxf(void *ptr);

#endif /* APXF_H */