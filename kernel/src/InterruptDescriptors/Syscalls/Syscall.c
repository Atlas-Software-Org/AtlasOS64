#include "../Syscall.h"
#include <Regs.h>
#include <stdint.h>

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
            break;
    }

    x64WriteRegs(&__proc_regs);
}
