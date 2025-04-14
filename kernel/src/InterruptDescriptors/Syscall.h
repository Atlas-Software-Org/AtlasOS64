#pragma once

struct InterruptFrame;

__attribute__((interrupt)) void SyscallInt_Hndlr(struct InterruptFrame* frame);

#define SYS_FSOPEN 0
#define SYS_FSCLOSE 1
#define SYS_FSWRITE 2
#define SYS_FSREAD 3
#define SYS_FSSEEK 4
#define SYS_FSRENAME 5
#define SYS_FSDELETE 6
#define SYS_FSGETSIZE 7
#define SYS_RDOPEN 8
#define SYS_RDCLOSE 9
#define SYS_RDREAD 11
#define SYS_WRITE 12
#define SYS_READ 13
#define SYS_THREADSET 14
#define SYS_THREADUNSET 15
#define SYS_THREADSCHED 16
#define SYS_THREADYIELD 17
#define SYS_gpx1IOWRITE 18
#define SYS_gpx1IOREAD 19
#define SYS_gpx1WINCREATE 20
#define SYS_MMAP 21
#define SYS_UMAP 22
#define SYS_THREADCREATEPROC 23
#define SYS_TERMINATEPROC 24
#define SYS_EXITPROC 25