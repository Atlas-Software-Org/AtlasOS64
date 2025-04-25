#pragma once

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