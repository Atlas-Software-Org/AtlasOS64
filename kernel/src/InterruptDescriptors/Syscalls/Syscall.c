#include "../Syscall.h"
#include <stdint.h>

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t rflags;
} x64Regs;

void x64WriteRegs(x64Regs *regs) {
    asm volatile(
        "mov %0, %%rax\n"   // Move value of regs->rax into rax register
        "mov %1, %%rbx\n"   // Move value of regs->rbx into rbx register
        "mov %2, %%rcx\n"   // Move value of regs->rcx into rcx register
        "mov %3, %%rdx\n"   // Move value of regs->rdx into rdx register
        "mov %4, %%rsi\n"   // Move value of regs->rsi into rsi register
        "mov %5, %%rdi\n"   // Move value of regs->rdi into rdi register
        "mov %6, %%r8\n"    // Move value of regs->r8 into r8 register
        "mov %7, %%r9\n"    // Move value of regs->r9 into r9 register
        "mov %8, %%r10\n"   // Move value of regs->r10 into r10 register
        "mov %9, %%r11\n"   // Move value of regs->r11 into r11 register
        "mov %10, %%r12\n"  // Move value of regs->r12 into r12 register
        "mov %11, %%r13\n"  // Move value of regs->r13 into r13 register
        "mov %12, %%r14\n"  // Move value of regs->r14 into r14 register
        "mov %13, %%r15\n"  // Move value of regs->r15 into r15 register
        : // No output operands
        : "m"(regs->rax), "m"(regs->rbx), "m"(regs->rcx), "m"(regs->rdx),
          "m"(regs->rsi), "m"(regs->rdi), "m"(regs->r8), "m"(regs->r9),
          "m"(regs->r10), "m"(regs->r11), "m"(regs->r12), "m"(regs->r13),
          "m"(regs->r14), "m"(regs->r15)
        : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", 
          "%r10", "%r11", "%r12", "%r13", "%r14", "%r15" // Clobbered registers
    );
}

void x64ReadRegs(x64Regs *regs) {
    asm volatile(
        "mov %%rax, %0\n"   // Move rax register value into regs->rax
        "mov %%rbx, %1\n"   // Move rbx register value into regs->rbx
        "mov %%rcx, %2\n"   // Move rcx register value into regs->rcx
        "mov %%rdx, %3\n"   // Move rdx register value into regs->rdx
        "mov %%rsi, %4\n"   // Move rsi register value into regs->rsi
        "mov %%rdi, %5\n"   // Move rdi register value into regs->rdi
        "mov %%r8, %6\n"    // Move r8 register value into regs->r8
        "mov %%r9, %7\n"    // Move r9 register value into regs->r9
        "mov %%r10, %8\n"   // Move r10 register value into regs->r10
        "mov %%r11, %9\n"   // Move r11 register value into regs->r11
        "mov %%r12, %10\n"  // Move r12 register value into regs->r12
        "mov %%r13, %11\n"  // Move r13 register value into regs->r13
        "mov %%r14, %12\n"  // Move r14 register value into regs->r14
        "mov %%r15, %13\n"  // Move r15 register value into regs->r15
        : // Outputs
        : "m"(regs->rax), "m"(regs->rbx), "m"(regs->rcx), "m"(regs->rdx),
          "m"(regs->rsi), "m"(regs->rdi), "m"(regs->r8), "m"(regs->r9),
          "m"(regs->r10), "m"(regs->r11), "m"(regs->r12), "m"(regs->r13),
          "m"(regs->r14), "m"(regs->r15)
        : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", 
          "%r10", "%r11", "%r12", "%r13", "%r14", "%r15" // Clobbered registers
    );
}

#define IOFSOpen 0
#define IOFSClose 1
#define IOFSWrite 2
#define IOFSRead 3
#define IOFSSeek 4
#define IOFSRename 5
#define IOFSDelete 6
#define IOFSGetSize 7
#define IORDOpen 8
#define IORDClose 9
#define IORDWrite 10
#define IORDRead 11
#define IOConWrite 12
#define IOConRead 13
#define HTThreadSet 14
#define HTThreadUnset 15
#define HTThreadsSched 16
#define HTThreadsYield 17
#define gpx1IOWrite 18
#define gpx1IORead 19
#define gpx1winCreate 20
#define IOMMap 21
#define IOMUMAP 22
#define HTThreadsCreateProc 23
#define HTThreadsTerminateProc 24 // exit on error
#define HTThreadsExitProc 25 // exit on finished

#include <HtKernelUtils/debug.h>
#include <gpx1.h>
__attribute__((interrupt)) void SyscallInt_Hndlr(struct InterruptFrame* Frame) {
    x64Regs __proc_regs;
    x64ReadRegs(&__proc_regs);
    x64Regs* regs = &__proc_regs;

    e9debugkf("Syscall: %d | 0x%x\n", regs->rdi, regs->rdi);
    switch (regs->rdi) { // Syscall number
        case IOFSOpen:
            // Syscall for opening a file
            break;

        case IOFSClose:
            // Syscall for closing a file
            break;

        case IOFSWrite:
            // Syscall for writing to a file
            break;

        case IOFSRead:
            // Syscall for reading from a file
            break;

        case IOFSSeek:
            // Syscall for seeking in a file
            break;

        case IOFSRename:
            // Syscall for renaming a file
            break;

        case IOFSDelete:
            // Syscall for deleting a file
            break;

        case IOFSGetSize:
            // Syscall for getting the size of a file
            break;

        case IORDOpen:
            // Syscall for opening a RamDisk
            break;

        case IORDClose:
            // Syscall for closing a RamDisk
            break;

        case IORDWrite:
            // Syscall for writing to a RamDisk
            break;

        case IORDRead:
            // Syscall for reading from a RamDisk
            break;

        case IOConWrite:
            // Syscall for writing to the console
            break;

        case IOConRead:
            // Syscall for reading from the console
            break;

        case HTThreadSet:
            // Syscall for setting a thread for execution
            break;

        case HTThreadUnset:
            // Syscall for marking a thread as unused
            break;

        case HTThreadsSched:
            // Syscall for scheduling threads for execution
            break;

        case HTThreadsYield:
            // Syscall for yielding the CPU to another thread
            break;

        case gpx1IOWrite:
            uint64_t ReqX = regs->rsi;
            uint64_t ReqY = regs->rdx;
            uint32_t Clr = regs->r10;
            PutPx(ReqX, ReqY, Clr);
            break;
        case gpx1IORead:
            void* addrx = (void*)regs->rsi;
            void* addry = (void*)regs->rdx;
            void* addrclr = (void*)regs->r10;
            // Dereference the passed addresses
            uint64_t* x = (uint64_t*)addrx;
            uint64_t* y = (uint64_t*)addry;
            uint64_t* clr = (uint64_t*)addrclr;
            
            // Get the pixel color at the coordinates (x, y)
            *clr = GetPx(*x, *y);
            break;

        case gpx1winCreate:
            // Syscall for creating a graphical window
            break;

        case IOMMap:
            // Syscall for mapping memory (mapping pages)
            break;

        case IOMUMAP:
            // Syscall for unmapping memory (unmapping pages)
            break;

        case HTThreadsCreateProc:
            // Syscall for creating a new process
            break;

        case HTThreadsTerminateProc:
            // Syscall for terminating a process (exit on error)
            break;

        case HTThreadsExitProc:
            // Syscall for exiting a process (finished)
            break;

        default:
            e9debugkf("Invalid syscall: %d | 0x%x\n", regs->rdi, regs->rdi);
            break;
    }

    x64WriteRegs(&__proc_regs);
}
