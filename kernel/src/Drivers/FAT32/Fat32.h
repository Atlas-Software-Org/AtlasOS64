#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem/mem.h>
#include <memory/heap.h>
#include <Drivers/ATA/ATA.h>

#define FAT_CLUSTER_FREE 0x00000000
#define FAT_CLUSTER_END  0x0FFFFFFF

#define FAT_SEEK_SET 0
#define FAT_SEEK_CUR 1
#define FAT_SEEK_END 2

#define FAT32_SECTOR_SIZE     512
#define FAT32_MAX_HANDLES     512

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_CREAT         0x0040
#define O_EXCL          0x0080
#define O_NOCTTY        0x0100
#define O_TRUNC         0x0200
#define O_APPEND        0x0400
#define O_NONBLOCK      0x0800
#define O_SYNC          0x101000
#define O_DSYNC         0x1000
#define O_RSYNC         0x101000
#define O_DIRECTORY     0x20000
#define O_NOFOLLOW      0x40000
#define O_CLOEXEC       0x80000

typedef struct __attribute__((packed)) {
    uint8_t  BS_jmpBoot[3];      // Jump instruction to boot code
    uint8_t  BS_OEMName[8];      // OEM Name in ASCII
    uint16_t BPB_BytsPerSec;     // Bytes per sector (usually 512)
    uint8_t  BPB_SecPerClus;     // Sectors per cluster
    uint16_t BPB_RsvdSecCnt;     // Reserved sectors count
    uint8_t  BPB_NumFATs;        // Number of FAT tables
    uint16_t BPB_RootEntCnt;     // Root directory entries (FAT12/16)
    uint16_t BPB_TotSec16;       // Total sectors (if zero, use TotSec32)
    uint8_t  BPB_Media;          // Media descriptor type
    uint16_t BPB_FATSz16;        // Sectors per FAT for FAT12/16
    uint16_t BPB_SecPerTrk;      // Sectors per track (for BIOS)
    uint16_t BPB_NumHeads;       // Number of heads (for BIOS)
    uint32_t BPB_HiddSec;        // Hidden sectors before partition
    uint32_t BPB_TotSec32;       // Total sectors (if TotSec16 is zero)

    // FAT32-specific
    uint32_t BPB_FATSz32;        // Sectors per FAT
    uint16_t BPB_ExtFlags;       // Extended flags
    uint16_t BPB_FSVer;          // File system version
    uint32_t BPB_RootClus;       // Root directory first cluster
    uint16_t BPB_FSInfo;         // FSInfo sector
    uint16_t BPB_BkBootSec;      // Backup boot sector
    uint8_t  BPB_Reserved[12];   // Reserved
    uint8_t  BS_DrvNum;          // Drive number
    uint8_t  BS_Reserved1;       // Reserved
    uint8_t  BS_BootSig;         // Extended boot signature (0x29)
    uint32_t BS_VolID;           // Volume serial number
    uint8_t  BS_VolLab[11];      // Volume label in ASCII
    uint8_t  BS_FilSysType[8];   // File system type string ("FAT32   ")
} FAT32_BPB;

typedef struct {
    bool  used;
    uint32_t firstCluster;
    uint32_t position;
    uint32_t size;
    uint8_t buf[512];
    uint32_t bufLBA;
    int flags;
} FileHandle;

struct FAT_DirEntry {
    char name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t createTimeTenths;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t accessDate;
    uint16_t clusterHigh;
    uint16_t modifyTime;
    uint16_t modifyDate;
    uint16_t clusterLow;
    uint32_t size;
    int used;
} __attribute__((packed));

bool FAT_Init(void);
int FAT_Open(const char *filename, uint32_t flags);
int FAT_Read(int fileHandle, void *buffer, size_t count);
int FAT_Close(int fileHandle);

int FAT_lseek(int fh, uint32_t offset, int whence);
int FAT_seek(FileHandle* fh, uint32_t offset);
int FAT_tell(int fh);

#endif // FAT32_H
