#pragma once

#include <stddef.h>
#include <stdint.h>
#include <mem/mem.h>
#include <paging/paging.h>

typedef struct {
    void* __ptr;
    size_t __len;
    int __open;
} RamDiskFile;

void LoadRamDiskImg(void* ptr, size_t size);
RamDiskFile* RamDiskOpen(char filename[12]);
void RamDiskClose(RamDiskFile* file);
void RamDiskRead(RamDiskFile* file, void* ptr, size_t* len);