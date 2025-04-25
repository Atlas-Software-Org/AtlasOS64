#include "RamDisk.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

uint8_t Address_U8_TEMP[16 * 1024 * 1024] = {0};
void* RamDiskPtr;
const uint64_t SizeofRamDisk = 16 * 1024 * 1024;

static bool IsValidRDChar(char c) {
    return (c >= '0' && c <= '9') ||   // Check for numbers
           (c >= 'a' && c <= 'z') ||   // Check for lowercase letters
           (c >= 'A' && c <= 'Z') ||   // Check for uppercase letters
           (c == '.') ||               // Check for dot
           (c == '_') ||               // Check for underscore
           (c == '-');                 // Check for dash
}

void InitRD() {
    RamDiskPtr = (void*)&Address_U8_TEMP;

    CreateRDDirectory("/");
}

RamDiskDirectory* GetRDRootDirectory() {
    return FindRDDirectory("/");
}

RamDiskFile OpenRamDiskFileHandles[512];
int CountOpen = 0;

int CompareNames(char a[12], char b[12]) {
    for (int i = 0; i < 12; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

#include <HtKernelUtils/debug.h>
RamDiskFile* OpenRDFile(char Filename[12], char Flag) {
    for (uint64_t offset = 0; offset + sizeof(RamDiskFile) <= SizeofRamDisk; offset += sizeof(RamDiskFile)) {
        RamDiskFile* file = (RamDiskFile*)((uint8_t*)RamDiskPtr + offset);
        if (CompareNames(file->Name, Filename)) {
            if (CountOpen >= 512) return NULL;
            int slot = -1;
            for (int i = 0; i < 512; i++) {
                if (!OpenRamDiskFileHandles[i].Open) {
                    slot = i;
                    break;
                }
            }
            if (slot == -1) return NULL;

            OpenRamDiskFileHandles[slot] = *file;
            if (Flag == 'r') OpenRamDiskFileHandles[slot].Flags = RD_O_RDONLY;
            else if (Flag == 'w') OpenRamDiskFileHandles[slot].Flags = RD_O_WRONLY;
            else if (Flag == 'b') OpenRamDiskFileHandles[slot].Flags = RD_O_RWONLY;
            OpenRamDiskFileHandles[slot].Open = true;

            CountOpen++;
            return &OpenRamDiskFileHandles[slot];
        }
    }
    return NULL;
}

RamDiskFile* OpenRDFile2(const char* path, char Flag) {
    if (!path) return NULL;

    char dirname[12] = {0};
    char filename[12] = {0};

    // Split the path into directory and filename based on the first slash, if it exists
    int i = 0;
    while (path[i] && path[i] != '/' && i < 12) {
        dirname[i] = path[i];
        i++;
    }

    // If there is a slash, continue parsing the filename
    if (path[i] == '/') {
        i++;  // Skip the slash
        int j = 0;
        while (path[i] && j < 12) {
            filename[j++] = path[i++];
        }
    } else {
        // No slash, the entire path is the filename
        for (int k = 0; k < 12; k++) filename[k] = dirname[k];
        dirname[0] = '\0'; // No directory
    }

    // Find the directory if it was specified
    RamDiskDirectory* dir = NULL;
    if (dirname[0] != '\0') {
        dir = FindRDDirectory(dirname);
        if (!dir) return NULL;  // Directory not found
    }

    // Iterate through all RamDisk files to find the matching filename
    for (uint64_t offset = 0; offset + sizeof(RamDiskFile) <= SizeofRamDisk; offset += sizeof(RamDiskFile)) {
        RamDiskFile* file = (RamDiskFile*)((uint8_t*)RamDiskPtr + offset);
        
        // Compare the file name with the given filename
        if (CompareNames(file->Name, filename)) {
            if (dirname[0] != '\0' && file->ParentDirectory != dir) continue;

            // Check for available slots to open the file
            if (CountOpen >= 512) return NULL;

            int slot = -1;
            for (int k = 0; k < 512; k++) {
                if (!OpenRamDiskFileHandles[k].Open) {
                    slot = k;
                    break;
                }
            }
            if (slot == -1) return NULL;

            // Copy file to the open file handle slot
            OpenRamDiskFileHandles[slot] = *file;

            // Set flags based on the requested access mode
            if (Flag == 'r') OpenRamDiskFileHandles[slot].Flags = RD_O_RDONLY;
            else if (Flag == 'w') OpenRamDiskFileHandles[slot].Flags = RD_O_WRONLY;
            else if (Flag == 'b') OpenRamDiskFileHandles[slot].Flags = RD_O_RWONLY;

            OpenRamDiskFileHandles[slot].Open = true;
            CountOpen++;

            return &OpenRamDiskFileHandles[slot];
        }
    }

    return NULL;
}

void CloseRDFile(RamDiskFile* File) {
    for (int i = 0; i < 512; i++) {
        if (CompareNames(OpenRamDiskFileHandles[i].Name, File->Name)) {
            OpenRamDiskFileHandles[i].Open = false;

            CountOpen--;
            return;
        }
    }
}

int ReadRDFile(RamDiskFile* File, char* __out) {
    for (int i = 0; i < 512; i++) {
        if (CompareNames(OpenRamDiskFileHandles[i].Name, File->Name)) {
            if (OpenRamDiskFileHandles[i].Flags != RD_O_RDONLY &&
                OpenRamDiskFileHandles[i].Flags != RD_O_RWONLY)
                return -1;
            for (int j = 0; j < OpenRamDiskFileHandles[i].Len; j++) {
                __out[j] = OpenRamDiskFileHandles[i].Buf[j];
            }
            return OpenRamDiskFileHandles[i].Len;
        }
    }
    return -1;
}

int WriteRDFile(RamDiskFile* File, char* __buf, int len) {
    for (int i = 0; i < 512; i++) {
        if (CompareNames(OpenRamDiskFileHandles[i].Name, File->Name)) {
            if (OpenRamDiskFileHandles[i].Flags != RD_O_WRONLY &&
                OpenRamDiskFileHandles[i].Flags != RD_O_RWONLY)
                return -1;

            RamDiskFile* realFile = NULL;
            for (uint64_t offset = 0; offset + sizeof(RamDiskFile) <= SizeofRamDisk; offset += sizeof(RamDiskFile)) {
                RamDiskFile* ptr = (RamDiskFile*)((uint8_t*)RamDiskPtr + offset);
                if (CompareNames(ptr->Name, File->Name)) {
                    realFile = ptr;
                    break;
                }
            }

            if (!realFile) return -1;

            if (len > 2048) len = 2048;

            for (int j = 0; j < len; j++) {
                realFile->Buf[j] = __buf[j];
            }

            realFile->Len = len;

            OpenRamDiskFileHandles[i] = *realFile;
            OpenRamDiskFileHandles[i].Open = true;

            return len;
        }
    }
    return -1;
}

int CreateRDFile(RamDiskDirectory* ParentDirectory, char Name[12]) {
    if (FindRDDirectory(ParentDirectory->Name) == NULL) return -1;
    for (uint64_t offset = 0; offset + sizeof(RamDiskFile) <= SizeofRamDisk; offset += sizeof(RamDiskFile)) {
        RamDiskFile* file = (RamDiskFile*)((uint8_t*)RamDiskPtr + offset);
        if (file->Name[0] == '\0') {
            for (int i = 0; i < 12; i++) file->Name[i] = Name[i];
            file->ParentDirectory = ParentDirectory;
            file->Len = 0;
            file->Open = false;
            file->Flags = 0;
            return 1;
        }
    }
    return -1;
}

int CreateRDDirectory(char DirName[12]) {
    for (uint64_t offset = 0; offset + sizeof(RamDiskDirectory) <= SizeofRamDisk; offset += sizeof(RamDiskDirectory)) {
        RamDiskDirectory* dir = (RamDiskDirectory*)((uint8_t*)RamDiskPtr + offset);
        if (dir->Name[0] == '\0') {
            for (int i = 0; i < 12; i++) dir->Name[i] = DirName[i];
            dir->ParentDirectory = NULL;
            return 1;
        }
    }
    return -1;
}

RamDiskDirectory* FindRDDirectory(char DirName[12]) {
    for (uint64_t offset = 0; offset + sizeof(RamDiskDirectory) <= SizeofRamDisk; offset += sizeof(RamDiskDirectory)) {
        RamDiskDirectory* dir = (RamDiskDirectory*)((uint8_t*)RamDiskPtr + offset);
        if (CompareNames(dir->Name, DirName)) {
            return dir;
        }
    }
    return NULL;
}