#ifndef MEM_H
#define MEM_H 1

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

void SetLmMap(struct limine_memmap_response* mMap);
struct limine_memmap_response* GetLmMap();

uint64_t GetMemorySize(struct limine_memmap_response* mMap);

int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
char *strstr(const char *haystack, const char *needle);
char *strchr(const char *s, int c);
size_t strlen(const char *s);

#endif /* MEM_H */