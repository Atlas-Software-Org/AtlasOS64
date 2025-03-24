#include "RamDisk.h"

void* RamDiskPtr;
size_t ramDiskSize;

void LoadRamDiskImg(void* ptr, size_t size) {
    RamDiskPtr = ptr;
    ramDiskSize = size;
}

RamDiskFile* RamDiskOpen(char filename[12]) {
    int found = 0;
    void* ptr = NULL;
    size_t fileLen = 0;

    for (size_t i = 0; i < ramDiskSize; i++) {
        if (memcmp((uint8_t*) &(RamDiskPtr[i]), "\xAB\xFF", 2) == 0) {
            i += 2;
            if (memcmp((uint8_t*) &(RamDiskPtr[i]), filename, 12) == 0) {
                ptr = &RamDiskPtr[i+12];
                found = 1;
            }
        }

        if (memcmp((uint8_t*) &(RamDiskPtr[i]), "\xAB\xE0", 2) == 0 && found == 1) {
            fileLen = (size_t)((uint64_t)&RamDiskPtr[i] - (uint64_t)ptr);
            break;
        }
    }

    if (found != 1 || fileLen == 0) {
        return NULL;
    }

    size_t totalSize = sizeof(RamDiskFile) + fileLen;
    size_t numPages = (totalSize + 4095) / 4096;
    RamDiskFile* file = (RamDiskFile*)page_alloc_n(numPages);
    if (!file) {
        return NULL;
    }

    file->__ptr = ptr;
    file->__len = fileLen;
    file->__open = 1;
    return file;
}

void RamDiskClose(RamDiskFile* file) {
    if (file) {
        file->__open = 0;
        size_t totalSize = sizeof(RamDiskFile) + file->__len;
        
        // Ensure we always allocate at least one page
        if (totalSize == 0) {
            totalSize = 1;
        }

        size_t numPages = (totalSize + 4095) / 4096;
        page_free_n(file, numPages);
    }
}

void RamDiskRead(RamDiskFile* file, void* ptr, size_t* len) {
    if (file && ptr && len) {
        memcpy(ptr, file->__ptr, file->__len);
        *len = file->__len;
    }
}
