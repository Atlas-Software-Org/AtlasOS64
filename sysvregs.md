### x86_64 (64-bit) - System V ABI Register Order

In x86_64 (64-bit), system call arguments are passed through specific registers. The first six arguments to a system call are passed in the following registers:

| Argument # | Register |
|------------|----------|
| 1st        | rdi      |
| 2nd        | rsi      |
| 3rd        | rdx      |
| 4th        | r10      |
| 5th        | r8       |
| 6th        | r9       |

For arguments beyond the sixth one, the system typically uses the stack to pass them. The return value is stored in the rax register.

### System Call Convention (x86_64):
- rax: System call number.
- rdi, rsi, rdx, r10, r8, r9: Arguments 1 to 6.
- Return value: The return value of the system call is stored in rax.

Example:
If you are making a system call to write (syscall number 1 in Linux):
- rax will hold 1 (the syscall number for write).
- rdi will hold the file descriptor (first argument).
- rsi will hold the pointer to the buffer (second argument).
- rdx will hold the size of the buffer (third argument).


### x86 (32-bit) - System V ABI Register Order

In x86 (32-bit), system calls are handled using the int 0x80 instruction. The register usage for system calls is as follows:

| Argument # | Register |
|------------|----------|
| 1st        | eax      |
| 2nd        | ebx      |
| 3rd        | ecx      |
| 4th        | edx      |
| 5th        | esi      |
| 6th        | edi      |

The return value is stored in the eax register.

System Call Convention (x86):
- eax: System call number.
- ebx, ecx, edx, esi, edi: Arguments 1 to 5.
- Return value: The return value of the system call is stored in eax.

Example:
If you are making a system call to write (syscall number 4 in Linux):
- eax will hold 4 (the syscall number for write).
- ebx will hold the file descriptor (first argument).
- ecx will hold the pointer to the buffer (second argument).
- edx will hold the size of the buffer (third argument).
