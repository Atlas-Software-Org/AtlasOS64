#ifndef SCHED_H
#define SCHED_H 1

#include <Regs.h>
#include <memory/paging.h>
#include <mem/mem.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    void (*Handler)();
    void* StackAddr;
    uint64_t StackBase;
    uint64_t StackPointer;
    x64Regs* RegStates;
    uint64_t IP;
    uint64_t __reserved;
} Thread;

extern Thread threads[8192];

int CreateThread(Thread* thread);
void Sched_Yield();

#endif /* SCHED_H */