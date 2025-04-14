#ifndef _AXF_H
#define _AXF_H 1

#include <stdint.h>
#include <stddef.h>

#define LittleEndian 1
#define BigEndian 0

#define SizeofAxf64Hdr 43
#define SizeofAxf32Hdr 27

typedef struct {
    uint8_t AI_MAG[3];
    uint64_t AEntry;
    uint8_t AArch;
    uint8_t AVersion;
    uint16_t ATrgtOS;
    uint8_t AAbi;
    uint64_t CodeSegSize;
    uint64_t DataSegSize;
    uint64_t BssSegSize;
    uint8_t ABSS0;
    uint8_t EndHdrMag[2];
} __attribute__((packed)) AHdr64_t;

typedef union {
    AHdr64_t HdrStruc;
    uint8_t HdrBytes[SizeofAxf64Hdr];
} AHdr64_U;

typedef struct {
    uint8_t AI_MAG[3];
    uint32_t AEntry;
    uint8_t AArch;
    uint8_t AVersion;
    uint16_t ATrgtOS;
    uint8_t AAbi;
    uint32_t CodeSegSize;
    uint32_t DataSegSize;
    uint32_t BssSegSize;
    uint8_t ABSS0;
    uint8_t EndHdrMag[2];
} __attribute__((packed)) AHdr32_t;

typedef union {
    AHdr32_t HdrStruc;
    uint8_t HdrBytes[SizeofAxf32Hdr];
} AHdr32_U;

/* Magic Bytes */

#define AI_MAG0 'A'
#define AI_MAG1 'X'
#define AI_MAG2 'F'

/* Architecture */

#define AA_64 0x01
#define AA_32 0x00

/* Version */

#define AV_2025 0x01

/* Target OS */

#define AtlasOS64 0xFFFF

/* ABI */

#define AABI_SYSV 0xFF

/* BSS0 */

#define ABSS0_YES 0x01
#define ABSS0_NO  0x00

/* End Header Magic */

#define AEHDR_MAG 0xE0FA

#define AE_MAG 0xEEFFEEFF

#endif