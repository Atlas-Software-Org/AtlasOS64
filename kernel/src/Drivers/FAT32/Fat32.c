#include "Fat32.h"
#include <stdint.h>
#include <stdbool.h>

// --- Global pointers ---
uint8_t* fat_rootdir_ptr = 0;
uint32_t fat_rootdir_count = 0;

uint8_t* fat_fat_ptr = 0;
uint8_t* fat_cluster_begin_ptr = 0;

#define SECTOR_SIZE 512
#define MAX_OPENFILES 4096

static FAT32_BPB bpb;
static uint32_t dataLBA;
static uint8_t secPerClust;

static FileHandle handles[MAX_OPENFILES];

#define ROOT_DIR_ENTRIES 65536

struct FAT_DirEntry root_dir[ROOT_DIR_ENTRIES];

uint64_t cluster_count;

static int str_ncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if ((uint8_t)a[i] != (uint8_t)b[i]) return (uint8_t)a[i] - (uint8_t)b[i];
    }
    return 0;
}

static void str_ncpy(char *dest, const char *src, unsigned int n) {
    unsigned int i = 0;
    while (i < n && src[i]) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i++] = '\0';
    }
}

// FAT_strchr implementation
char *FAT_strchr(const char *str, int c) {
    while (*str != '\0') {   // Loop through each character in the string
        if (*str == (char)c)  // Check if current character matches 'c'
            return (char *)str; // Return the pointer to the matching character
        str++; // Move to the next character
    }
    return NULL; // Return NULL if no match is found
}

char *FAT_strtok(char *str, const char *delim) {
    static char *last = NULL;  // Static pointer to remember where we left off in the string
    char *start;

    // If str is NULL, continue from where we left off in the previous call
    if (str == NULL) {
        str = last;
    }

    // Skip leading delimiters
    if (str == NULL || *str == '\0') {
        return NULL;
    }

    // Find the start of the token
    start = str;

    // Move the pointer to the next delimiter or the end of the string
    while (*str != '\0' && FAT_strchr(delim, *str) == NULL) {
        str++;
    }

    // If we found a delimiter, null-terminate the current token and update 'last'
    if (*str != '\0') {
        *str = '\0'; // Null-terminate the token
        last = str + 1; // Point to the character after the delimiter for the next call
    } else {
        last = NULL;  // No more tokens
    }

    return start;
}

// FAT_strcpy implementation
char *FAT_strcpy(char *dst, const char *src) {
    char *d = dst;
    const char *s = src;

    // Copy characters from src to dst until we reach the null terminator
    while ((*d++ = *s++) != '\0');  // Copy character and move both pointers

    return dst;  // Return the destination pointer
}

static void readSector(uint32_t lba, void *buf) {
    g_DiskDriver->ReadSect(lba, buf);
}

bool FAT_Init(void) {
    uint8_t buf[SECTOR_SIZE];
    readSector(0, buf);
    memcpy(&bpb, buf, sizeof(FAT32_BPB));
    secPerClust = bpb.BPB_SecPerClus;
    uint32_t reserved = bpb.BPB_RsvdSecCnt;
    uint32_t fatsz = bpb.BPB_FATSz32;
    dataLBA = reserved + bpb.BPB_NumFATs * fatsz;
    for (int i = 0; i < MAX_OPENFILES; i++) handles[i].used = false;

    cluster_count = (g_DiskDriver->GetTotalSectors() - bpb.BPB_RsvdSecCnt - (bpb.BPB_NumFATs * bpb.BPB_FATSz32)) / bpb.BPB_SecPerClus;  
    return true;
}

static uint32_t clusterToLBA(uint32_t cl) {
    return dataLBA + (cl - 2) * secPerClust;
}

static uint32_t getFATEntry(uint32_t cl) {
    uint32_t ent = cl * 4;
    uint32_t sect = bpb.BPB_RsvdSecCnt + (ent / SECTOR_SIZE);
    uint8_t buf[SECTOR_SIZE];
    readSector(sect, buf);
    return ((uint32_t*)buf)[(ent % SECTOR_SIZE)/4] & 0x0FFFFFFF;
}

static int findHandle(void) {
    for (int i = 0; i < MAX_OPENFILES; i++) if (!handles[i].used) return i;
    return -1;
}

static void formatSFN(const char *name, char out[11]) {
    memset(out, ' ', 11);
    int i=0; while (name[i] && name[i] != '.' && i<8) out[i] = (char)((name[i]>='a'&&name[i]<='z')?name[i]-32:name[i]), i++;
    int j=8; if (name[i]=='.') { i++; for (int k=0;k<3 && name[i];k++) out[j++] = (char)((name[i]>='a'&&name[i]<='z')?name[i]-32:name[i]), i++; }
}

uint32_t FAT_AllocRootDirEntry(void) {
    for (uint32_t i = 0; i < bpb.BPB_RootEntCnt; i++) {
        uint8_t* entry = fat_rootdir_ptr + (i * 32);
        if (entry[0] == 0x00 || entry[0] == 0xE5) {
            fat_rootdir_count++;
            return i;
        }
    }
    return 0xFFFFFFFF;
}

uint32_t FAT_AllocCluster() {
    uint32_t cluster_count = bpb.BPB_FATSz32 * bpb.BPB_SecPerClus * 8;  // Number of clusters in the FAT32 system
    uint32_t *fat_table = (uint32_t *)(fat_fat_ptr);  // Address of the FAT table

    // Try to find a free cluster
    for (uint32_t i = 2; i < cluster_count; i++) {  // Start at 2 since 0 and 1 are reserved
        if (fat_table[i] == 0) {  // Check if the cluster is free (value 0 means free in FAT32)
            fat_table[i] = 0x0FFFFFFF;  // Mark the cluster as used (FAT32 style)
            return i;  // Return the allocated cluster
        }
    }

    return 0xFFFFFFFF;  // No free clusters found
}

void FAT_InitDirEntry(uint32_t idx, const char *name, uint32_t first_cluster) {
    uint8_t* entry = fat_rootdir_ptr + (idx * 32);
    for (int i = 0; i < 11; i++) entry[i] = (name[i] != 0) ? name[i] : ' ';
    entry[11] = 0x20;
    entry[26] = first_cluster & 0xFF;
    entry[27] = (first_cluster >> 8) & 0xFF;
    entry[20] = (first_cluster >> 16) & 0xFF;
    entry[21] = (first_cluster >> 24) & 0xFF;
    entry[28] = 0;
    entry[29] = 0;
    entry[30] = 0;
    entry[31] = 0;
}

uint8_t* FAT_GetClusterPointer(uint32_t cluster) {
    if (cluster < 2 || cluster >= cluster_count + 2) return 0;
    return fat_cluster_begin_ptr + ((cluster - 2) * (bpb.BPB_SecPerClus * 512));
}

void FAT_UpdateSizeInDirEntry(uint32_t fd, uint32_t newsize) {
    uint8_t* entry = fat_rootdir_ptr + (fd * 32);
    entry[28] = newsize & 0xFF;
    entry[29] = (newsize >> 8) & 0xFF;
    entry[30] = (newsize >> 16) & 0xFF;
    entry[31] = (newsize >> 24) & 0xFF;
}

int FAT_Open(const char *fname, uint32_t flags) {
    char sfn[11]; formatSFN(fname, sfn);
    uint8_t buf[SECTOR_SIZE];
    uint32_t rootLBA = bpb.BPB_RootClus;
    FileHandle *h;
    uint32_t cl = rootLBA;
    do {
        for (uint8_t i=0; i<secPerClust; i++) {
            readSector(clusterToLBA(cl)+i, buf);
            for (int off=0; off<SECTOR_SIZE; off+=32) {
                uint8_t first = buf[off];
                if (first==0x00) return -1;
                if (first==0xE5) continue;
                if ((buf[off+11]&0x0F)==0) {
                    if (!memcmp(&buf[off], sfn, 11)) {
                        int hidx = findHandle(); if (hidx<0) return -1;
                        h = &handles[hidx]; h->used=true;
                        uint32_t lo = *(uint16_t*)(buf+off+26);
                        uint32_t hi = *(uint16_t*)(buf+off+20);
                        h->firstCluster = lo | (hi<<16);
                        h->size = *(uint32_t*)(buf+off+28);
                        h->position=0;
                        h->bufLBA = 0xFFFFFFFF;
                        if (flags != O_RDONLY &&
                            flags != O_WRONLY &&
                            flags != O_RDWR &&
                            flags != O_TRUNC) {
                            return -1;
                        }
                        h->flags = flags;
                        return hidx;
                    }
                }
            }
        }
        cl = getFATEntry(cl);
    } while (cl < 0x0FFFFFF8);
    return -1;
}

int FAT_Read(int fh, void *dst, size_t cnt) {
    FileHandle *h = &handles[fh];
    if (h->flags != O_RDONLY && h->flags != O_RDWR) {
        return -1;
    }
    if (!h->used) return -1;
    uint8_t *d = dst;
    size_t left = cnt;
    while (left && h->position < h->size) {
        uint32_t secOff = (h->position % (secPerClust * SECTOR_SIZE)) / SECTOR_SIZE;
        uint32_t lba = clusterToLBA(h->firstCluster) + secOff;

        // Invalidate cache if the LBA is different
        if (lba != h->bufLBA) {
            readSector(lba, h->buf);
            h->bufLBA = lba;
        }

        uint32_t boff = h->position % SECTOR_SIZE;
        uint32_t chunk = SECTOR_SIZE - boff;
        if (chunk > left) chunk = left;
        if (chunk > h->size - h->position) chunk = h->size - h->position;
        memcpy(d, h->buf + boff, chunk);
        d += chunk;
        left -= chunk;
        h->position += chunk;

        if (boff + chunk >= SECTOR_SIZE && secOff + 1 >= secPerClust) {
            h->firstCluster = getFATEntry(h->firstCluster);
        }
    }
    return (d - (uint8_t*)dst);
}

int FAT_FindFile(const char* name) {
    for (int i = 0; i < ROOT_DIR_ENTRIES; i++) {
        if (!root_dir[i].used) continue;

        int match = 1;
        for (int j = 0; j < 11; j++) {
            if (name[j] == 0) {
                if (root_dir[i].name[j] != ' ') {
                    match = 0;
                    break;
                }
            } else if (root_dir[i].name[j] != name[j]) {
                match = 0;
                break;
            }
        }

        if (match) return i;
    }

    return -1;
}

int FAT_Close(int fh) {
    if (fh<0||fh>=MAX_OPENFILES) return -1;
    handles[fh].used = false;
    handles[fh].flags = 0;
    return 0;
}

int FAT_lseek(int fh, uint32_t offset, int whence) {
    FileHandle* _fh = &handles[fh];

    uint32_t new_pos;

    if (whence == FAT_SEEK_SET) {
        new_pos = offset;
    } else if (whence == FAT_SEEK_CUR) {
        new_pos = _fh->position + offset;
    } else if (whence == FAT_SEEK_END) {
        new_pos = _fh->size + offset;
    } else {
        return -1;
    }

    if (new_pos > _fh->size) {
        return -1;
    }

    _fh->position = new_pos;
    _fh->bufLBA = (uint32_t)-1;
    return new_pos;
}

int FAT_seek(FileHandle* fh, uint32_t offset) {
    if (offset > fh->size) {
        return -1;
    }

    fh->position = offset;
    fh->bufLBA = (uint32_t)-1;
    return offset;
}

int FAT_tell(int fh) {
    if (fh<0||fh>=MAX_OPENFILES) return -1;
    if (handles[fh].used == false) return -1;
    return handles[fh].position;
}

void FAT_rewind(int fh) {
    FAT_lseek(fh, 0, FAT_SEEK_SET);
}
