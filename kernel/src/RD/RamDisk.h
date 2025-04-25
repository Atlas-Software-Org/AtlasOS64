#pragma once

#include <stddef.h>
#include <stdint.h>
#include <mem/mem.h>
#include <memory/paging.h>

#define SIZE_OF_RAMDISK (16 * 1024 * 1024)

void InitRD();

typedef struct RamDiskDirectory RamDiskDirectory;

typedef struct {
    char Name[12];
    RamDiskDirectory* ParentDirectory;
    char Buf[2048];
    uint8_t Flags;
    bool Open;
    uint16_t Len;
} RamDiskFile;

struct RamDiskDirectory {
    char Name[12];
    RamDiskDirectory* ParentDirectory;
    uint8_t reserved[2064];
};

typedef RamDiskDirectory RamDiskRootDirectory;

#define RD_O_WRONLY 0b00000001
#define RD_O_RDONLY 0b00000010
#define RD_O_RWONLY 0b00000100

// ---------------------- File handle I/O -----------------

RamDiskFile* OpenRDFile(char Filename[12], char Flag);
RamDiskFile* OpenRDFile2(const char* path, char Flag);
void CloseRDFile(RamDiskFile* File);

// ---------------------- File I/O ------------------------

int ReadRDFile(RamDiskFile* File, char* __out);
int WriteRDFile(RamDiskFile* File, char* __buf, int len);

// ---------------------- Entry creation ------------------

int CreateRDFile(RamDiskDirectory* ParentDirectory, char Name[12]);
int CreateRDDirectory(char DirName[12]);

// ---------------------- Directory I/O -------------------

RamDiskDirectory* FindRDDirectory(char DirName[12]);
RamDiskDirectory* GetRDRootDirectory();