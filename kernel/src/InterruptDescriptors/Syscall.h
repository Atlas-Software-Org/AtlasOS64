#pragma once

#include <stddef.h>

/* Syscall helper headers */

#include <Drivers/FAT32/Fat32.h>

/* end */

typedef long off_t;
typedef int pid_t;
typedef long ssize_t;
typedef unsigned long dev_t;
typedef unsigned long ino_t;
typedef unsigned short mode_t;
typedef unsigned long nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long time_t;
typedef long blksize_t;
typedef long blkcnt_t;
typedef unsigned int id_t;

struct stat {
    dev_t     st_dev;     /* ID of device containing file */
    ino_t     st_ino;     /* inode number */
    mode_t    st_mode;    /* protection (file type and access permissions) */
    nlink_t   st_nlink;   /* number of hard links */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    dev_t     st_rdev;    /* device ID (if special file) */
    off_t     st_size;    /* total size, in bytes */
    time_t    st_atime;   /* time of last access */
    time_t    st_mtime;   /* time of last modification */
    time_t    st_ctime;   /* time of last status change */
    blksize_t st_blksize; /* preferred I/O block size for filesystem */
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
};

struct utimbuf {
    time_t actime;  /* access time */
    time_t modtime; /* modification time */
};

struct timespec {
    time_t tv_sec;  /* seconds */
    long   tv_nsec; /* nanoseconds */
};

typedef struct siginfo_t {
    int si_signo;  /* Signal number */
    int si_errno;  /* An error number (if applicable) */
    int si_code;   /* Signal code (specific information about signal) */
    pid_t si_pid;  /* Sending process ID */
    uid_t si_uid;  /* Sending user ID */
    int si_status; /* Exit status or signal (for SIGCHLD) */
    void *si_addr; /* Address at which the signal was received */
    long si_band;  /* Band event for SIGPOLL */
    int si_fd;     /* File descriptor for SIGPOLL */
} siginfo_t;

struct InterruptFrame;

#define SYS_open        0
#define SYS_close       1
#define SYS_read        2
#define SYS_write       3
#define SYS_pread       4
#define SYS_pwrite      5
#define SYS_lseek       6
#define SYS_fsync       7
#define SYS_fdatasync   8
#define SYS_dup         9
#define SYS_dup2        10
#define SYS_pipe        11
#define SYS_pipe2       12
#define SYS_truncate    13
#define SYS_ftruncate   14
#define SYS_rename      15
#define SYS_link        16
#define SYS_symlink     17
#define SYS_readlink    18
#define SYS_stat        19
#define SYS_lstat       20
#define SYS_fstat       21
#define SYS_chmod       22
#define SYS_fchmod      23
#define SYS_fchmodat    24
#define SYS_mkdir       25
#define SYS_rmdir       26
#define SYS_chdir       27
#define SYS_fchdir      28
#define SYS_getcwd      29
#define SYS_access      30
#define SYS_faccessat   31
#define SYS_utime       32
#define SYS_utimensat   33
#define SYS_futimens    34
#define SYS_mkfifo      35
#define SYS_mkfifoat    36
#define SYS_mknod       37
#define SYS_mknodat     38
#define SYS_fork        39
#define SYS_execve      40
#define SYS_execv       41
#define SYS_execvp      42
#define SYS_execvpe     43
#define SYS_exit        44
#define SYS_wait        45
#define SYS_waitpid     46
#define SYS_waitid      47
#define SYS_kill        48
#define SYS_getpid      49
#define SYS_getppid     50
#define SYS_getuid      51
#define SYS_geteuid     52
#define SYS_getgid      53
#define SYS_getegid     54
#define SYS_setuid      55
#define SYS_seteuid     56
#define SYS_setgid      57
#define SYS_setegid     58
#define SYS_setpgid     59
#define SYS_getpgid     60
#define SYS_getpgrp     61
#define SYS_setpgrp     62
#define SYS_setsid      63
#define SYS_getsid      64

// Function prototypes for syscall implementations
int sys_open(const char* path, int flags);
int sys_close(int fd);
ssize_t sys_read(int fd, void* buf, size_t count);
ssize_t sys_write(int fd, const void* buf, size_t count);
ssize_t sys_pread(int fd, void* buf, size_t count, off_t offset);
ssize_t sys_pwrite(int fd, const void* buf, size_t count, off_t offset);
off_t sys_lseek(int fd, off_t offset, int whence);
int sys_fsync(int fd);
int sys_fdatasync(int fd);
int sys_dup(int fd);
int sys_dup2(int oldfd, int newfd);
int sys_pipe(int pipefd[2]);
int sys_pipe2(int pipefd[2], int flags);
int sys_truncate(const char* path, off_t length);
int sys_ftruncate(int fd, off_t length);
int sys_rename(const char* oldpath, const char* newpath);
int sys_link(const char* oldpath, const char* newpath);
int sys_symlink(const char* target, const char* linkpath);
ssize_t sys_readlink(const char* path, char* buf, size_t bufsiz);
int sys_stat(const char* path, struct stat* statbuf);
int sys_lstat(const char* path, struct stat* statbuf);
int sys_fstat(int fd, struct stat* statbuf);
int sys_chmod(const char* path, mode_t mode);
int sys_fchmod(int fd, mode_t mode);
int sys_fchmodat(int dirfd, const char* path, mode_t mode);
int sys_mkdir(const char* path, mode_t mode);
int sys_rmdir(const char* path);
int sys_chdir(const char* path);
int sys_fchdir(int fd);
char* sys_getcwd(char* buf, size_t size);
int sys_access(const char* path, int mode);
int sys_faccessat(int dirfd, const char* path, int mode);
int sys_utime(const char* path, const struct utimbuf* times);
int sys_utimensat(int dirfd, const char* path, const struct timespec times[2], int flags);
int sys_futimens(int fd, const struct timespec times[2]);
int sys_mkfifo(const char* path, mode_t mode);
int sys_mkfifoat(int dirfd, const char* path, mode_t mode);
int sys_mknod(const char* path, mode_t mode, dev_t dev);
int sys_mknodat(int dirfd, const char* path, mode_t mode, dev_t dev);
pid_t sys_fork(void);
int sys_execve(const char* pathname, char* const argv[], char* const envp[]);
int sys_execv(const char* pathname, char* const argv[]);
int sys_execvp(const char* file, char* const argv[]);
int sys_execvpe(const char* file, char* const argv[], char* const envp[]);
void sys_exit(int status);
pid_t sys_wait(int* status);
pid_t sys_waitpid(pid_t pid, int* status, int options);
int sys_waitid(int idtype, id_t id, siginfo_t* info, int options);
int sys_kill(pid_t pid, int sig);
pid_t sys_getpid(void);
pid_t sys_getppid(void);
uid_t sys_getuid(void);
uid_t sys_geteuid(void);
gid_t sys_getgid(void);
gid_t sys_getegid(void);
int sys_setuid(uid_t uid);
int sys_seteuid(uid_t euid);
int sys_setgid(gid_t gid);
int sys_setegid(gid_t egid);
int sys_setpgid(pid_t pid, pid_t pgid);
pid_t sys_getpgid(pid_t pid);
pid_t sys_getpgrp(void);
int sys_setpgrp(void);
int sys_setsid(void);
pid_t sys_getsid(pid_t pid);

long syscall_dispatcher(long syscall_num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);