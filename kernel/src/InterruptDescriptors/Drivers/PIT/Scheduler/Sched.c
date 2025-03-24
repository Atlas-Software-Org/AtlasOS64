#include "Sched.h"
#include <HtKernelUtils/io.h>

Thread threads[8192];
size_t ThreadIdx = 0;

int CreateThread(Thread* thread) {
    if (ThreadIdx >= 8192) {
        return -1; // Unavailable threads
    }
    thread->__reserved = ThreadIdx;
    threads[ThreadIdx++] = *thread;
    return 0;
}

void Sched_nop() {
    asm volatile("nop");
}

void save_thread_state(Thread* thread) {
    x64Regs regs;
    x64ReadRegs(&regs);

    // Allocate memory for RegStates using page_alloc if not already done
    if (!thread->RegStates) {
        thread->RegStates = page_alloc(sizeof(x64Regs));  // Allocate a single page for RegStates
        if (!thread->RegStates) {
            // Handle memory allocation failure (return or log error)
            return;
        }
    }

    // Save the register state and current stack pointer
    memcpy(thread->RegStates, &regs, sizeof(x64Regs)); // Copy the register state to thread
    thread->StackPointer = regs.rsp;
    thread->StackBase = regs.rbp;

    // Store the Instruction Pointer (RIP)
    asm volatile("lea (%%rip), %0" : "=r"(thread->IP));  // Get current instruction pointer
}

void load_thread_state(Thread* thread) {
    // Check if RegStates is valid
    if (!thread->RegStates) {
        // Handle error: Invalid RegStates pointer
        return;
    }

    // Dereference the pointer to get the actual regs
    x64Regs regs = *thread->RegStates;

    // Load the registers from the saved state
    asm volatile(
        "mov %0, %%rax;"
        "mov %1, %%rbx;"
        "mov %2, %%rcx;"
        "mov %3, %%rdx;"
        "mov %4, %%rdi;"
        "mov %5, %%rsi;"
        "mov %6, %%rbp;"
        "mov %7, %%rsp;"
        "mov %8, %%r8;"
        "mov %9, %%r9;"
        "mov %10, %%r10;"
        "mov %11, %%r11;"
        "mov %12, %%r12;"
        "mov %13, %%r13;"
        "mov %14, %%r14;"
        "mov %15, %%r15;"
        : 
        : "m"(regs.rax), "m"(regs.rbx), "m"(regs.rcx), "m"(regs.rdx),
          "m"(regs.rdi), "m"(regs.rsi), "m"(regs.rbp), "m"(regs.rsp),
          "m"(regs.r8), "m"(regs.r9), "m"(regs.r10), "m"(regs.r11),
          "m"(regs.r12), "m"(regs.r13), "m"(regs.r14), "m"(regs.r15)
    );

    // Restore the Instruction Pointer (RIP)
    asm volatile(
        "jmp *%0;"  // Jump to the stored RIP (instruction pointer)
        : 
        : "r"(thread->IP)
    );
}

size_t CurrentlyRunningThread = 0;
void Sched_Yield() {
    save_thread_state(&threads[CurrentlyRunningThread]);
    CurrentlyRunningThread = (CurrentlyRunningThread + 1) % ThreadIdx;
    if (threads[CurrentlyRunningThread].Handler == Sched_nop) {
        return;
    }

    load_thread_state(&threads[CurrentlyRunningThread]);
}
