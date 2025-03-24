#include <Regs.h>

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