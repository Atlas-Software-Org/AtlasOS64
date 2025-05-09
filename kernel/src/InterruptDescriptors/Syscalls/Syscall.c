#include "../Syscall.h"
#include <Regs.h>
#include <stdint.h>

static inline long syscall0(long num) {
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall1(long num, long arg1) {
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall2(long num, long arg1, long arg2) {
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall3(long num, long arg1, long arg2, long arg3) {
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
    register long r10 asm("r10") = arg4;
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall5(long num, long arg1, long arg2, long arg3, long arg4, long arg5) {
    register long r10 asm("r10") = arg4;
    register long r8  asm("r8")  = arg5;
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall6(long num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    register long r10 asm("r10") = arg4;
    register long r8  asm("r8")  = arg5;
    register long r9  asm("r9")  = arg6;
    long ret;
    asm volatile ("int 0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

// Dispatcher function
long syscall_dispatcher(long syscall_num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    switch (syscall_num) {
        case SYS_open:
            return sys_open((const char*)arg1, (int)arg2);
        case SYS_close:
            return sys_close((int)arg1);
        case SYS_read:
            return sys_read((int)arg1, (void*)arg2, (size_t)arg3);
        case SYS_write:
            return sys_write((int)arg1, (const void*)arg2, (size_t)arg3);
        case SYS_pread:
            return sys_pread((int)arg1, (void*)arg2, (size_t)arg3, (off_t)arg4);
        case SYS_pwrite:
            return sys_pwrite((int)arg1, (const void*)arg2, (size_t)arg3, (off_t)arg4);
        case SYS_lseek:
            return sys_lseek((int)arg1, (off_t)arg2, (int)arg3);
        case SYS_fsync:
            return sys_fsync((int)arg1);
        case SYS_fdatasync:
            return sys_fdatasync((int)arg1);
        case SYS_dup:
            return sys_dup((int)arg1);
        case SYS_dup2:
            return sys_dup2((int)arg1, (int)arg2);
        case SYS_pipe:
            return sys_pipe((int*)arg1);
        case SYS_pipe2:
            return sys_pipe2((int*)arg1, (int)arg2);
        case SYS_truncate:
            return sys_truncate((const char*)arg1, (off_t)arg2);
        case SYS_ftruncate:
            return sys_ftruncate((int)arg1, (off_t)arg2);
        case SYS_rename:
            return sys_rename((const char*)arg1, (const char*)arg2);
        case SYS_link:
            return sys_link((const char*)arg1, (const char*)arg2);
        case SYS_symlink:
            return sys_symlink((const char*)arg1, (const char*)arg2);
        case SYS_readlink:
            return sys_readlink((const char*)arg1, (char*)arg2, (size_t)arg3);
        case SYS_stat:
            return sys_stat((const char*)arg1, (struct stat*)arg2);
        case SYS_lstat:
            return sys_lstat((const char*)arg1, (struct stat*)arg2);
        case SYS_fstat:
            return sys_fstat((int)arg1, (struct stat*)arg2);
        case SYS_chmod:
            return sys_chmod((const char*)arg1, (mode_t)arg2);
        case SYS_fchmod:
            return sys_fchmod((int)arg1, (mode_t)arg2);
        case SYS_fchmodat:
            return sys_fchmodat((int)arg1, (const char*)arg2, (mode_t)arg3);
        case SYS_mkdir:
            return sys_mkdir((const char*)arg1, (mode_t)arg2);
        case SYS_rmdir:
            return sys_rmdir((const char*)arg1);
        case SYS_chdir:
            return sys_chdir((const char*)arg1);
        case SYS_fchdir:
            return sys_fchdir((int)arg1);
        case SYS_getcwd:
            return (long)sys_getcwd((char*)arg1, (size_t)arg2);
        case SYS_access:
            return sys_access((const char*)arg1, (int)arg2);
        case SYS_faccessat:
            return sys_faccessat((int)arg1, (const char*)arg2, (int)arg3);
        case SYS_utime:
            return sys_utime((const char*)arg1, (const struct utimbuf*)arg2);
        case SYS_utimensat:
            return sys_utimensat((int)arg1, (const char*)arg2, (const struct timespec*)arg3, (int)arg4);
        case SYS_futimens:
            return sys_futimens((int)arg1, (const struct timespec*)arg2);
        case SYS_mkfifo:
            return sys_mkfifo((const char*)arg1, (mode_t)arg2);
        case SYS_mkfifoat:
            return sys_mkfifoat((int)arg1, (const char*)arg2, (mode_t)arg3);
        case SYS_mknod:
            return sys_mknod((const char*)arg1, (mode_t)arg2, (dev_t)arg3);
        case SYS_fork:
            return sys_fork();
        case SYS_execve:
            return sys_execve((const char*)arg1, (char* const*)arg2, (char* const*)arg3);
        case SYS_execv:
            return sys_execv((const char*)arg1, (char* const*)arg2);
        case SYS_execvp:
            return sys_execvp((const char*)arg1, (char* const*)arg2);
        case SYS_execvpe:
            return sys_execvpe((const char*)arg1, (char* const*)arg2, (char* const*)arg3);
        case SYS_exit:
            sys_exit((int)arg1);
            return 0;  // Exit the process
        case SYS_wait:
            return sys_wait((int*)arg1);
        case SYS_waitpid:
            return sys_waitpid((pid_t)arg1, (int*)arg2, (int)arg3);
        case SYS_waitid:
            return sys_waitid((int)arg1, (id_t)arg2, (siginfo_t*)arg3, (int)arg4);
        case SYS_kill:
            return sys_kill((pid_t)arg1, (int)arg2);
        case SYS_getpid:
            return sys_getpid();
        case SYS_getppid:
            return sys_getppid();
        case SYS_getuid:
            return sys_getuid();
        case SYS_geteuid:
            return sys_geteuid();
        case SYS_getgid:
            return sys_getgid();
        case SYS_getegid:
            return sys_getegid();
        case SYS_setuid:
            return sys_setuid((uid_t)arg1);
        case SYS_seteuid:
            return sys_seteuid((uid_t)arg1);
        case SYS_setgid:
            return sys_setgid((gid_t)arg1);
        case SYS_setegid:
            return sys_setegid((gid_t)arg1);
        case SYS_setpgid:
            return sys_setpgid((pid_t)arg1, (pid_t)arg2);
        case SYS_getpgid:
            return sys_getpgid((pid_t)arg1);
        case SYS_getpgrp:
            return sys_getpgrp();
        case SYS_setpgrp:
            return sys_setpgrp();
        case SYS_setsid:
            return sys_setsid();
        case SYS_getsid:
            return sys_getsid((pid_t)arg1);
        default:
            return -1;  // Unknown syscall
    }
}
