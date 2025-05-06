#include "../../Syscall.h"

int sys_open(const char* path, int flags) {
    return FAT_Open(path, flags);
}

int sys_close(int fd) {
    FAT_Close(fd);
    return 0;
}

ssize_t sys_read(int fd, void* buf, size_t count) {
    ssize_t bytes = FAT_Read(fd, buf, count);
    return bytes;
}

ssize_t sys_write(int fd, const void* buf, size_t count) {
    char* __buf = (char*)buf;
    ssize_t cnt = 0;
    switch (fd) {
        case 0: /* STDOUT */
            for (int i = 0; i < count && __buf[i] != 0; i++) {
                tty_putchar(__buf[i]);
                cnt++;
            }
            break;
        case 1: /* STDERR */
            for (int i = 0; i < count && __buf[i] != 0; i++) {
                tty_putchar(__buf[i]);
                e9debugkf("%c", __buf[i]);
                cnt++;
            }
            break;
        default:
            break;
    }
    return cnt;
}

ssize_t sys_pread(int fd, void* buf, size_t count, off_t offset) {
    FAT_lseek(fd, offset, FAT_SEEK_SET);
    ssize_t bytes = FAT_Read(fd, buf, count);
    FAT_lseek(fd, 0, FAT_SEEK_SET);
    return bytes;
}

ssize_t sys_pwrite(int fd, const void* buf, size_t count, off_t offset) {
    return 0;
}

off_t sys_lseek(int fd, off_t offset, int whence) {
    return FAT_lseek(fd,offset, whence);
}

int sys_fsync(int fd) {
    return 0;
}

int sys_fdatasync(int fd) {
    return 0;
}

int sys_dup(int fd) {

}

int sys_dup2(int oldfd, int newfd) {

}

int sys_pipe(int pipefd[2]) {

}

int sys_pipe2(int pipefd[2], int flags) {

}

int sys_truncate(const char* path, off_t length) {

}

int sys_ftruncate(int fd, off_t length) {

}

int sys_rename(const char* oldpath, const char* newpath) {

}

int sys_link(const char* oldpath, const char* newpath) {

}

int sys_symlink(const char* target, const char* linkpath) {

}

ssize_t sys_readlink(const char* path, char* buf, size_t bufsiz) {

}

int sys_stat(const char* path, struct stat* statbuf) {

}

int sys_lstat(const char* path, struct stat* statbuf) {

}

int sys_fstat(int fd, struct stat* statbuf) {

}

int sys_chmod(const char* path, mode_t mode) {

}

int sys_fchmod(int fd, mode_t mode) {

}

int sys_fchmodat(int dirfd, const char* path, mode_t mode) {

}

int sys_mkdir(const char* path, mode_t mode) {

}

int sys_rmdir(const char* path) {

}

int sys_chdir(const char* path) {

}

int sys_fchdir(int fd) {

}

char* sys_getcwd(char* buf, size_t size) {

}

int sys_access(const char* path, int mode) {

}

int sys_faccessat(int dirfd, const char* path, int mode) {

}

int sys_utime(const char* path, const struct utimbuf* times) {

}

int sys_utimensat(int dirfd, const char* path, const struct timespec times[2], int flags) {

}

int sys_futimens(int fd, const struct timespec times[2]) {

}

int sys_mkfifo(const char* path, mode_t mode) {

}

int sys_mkfifoat(int dirfd, const char* path, mode_t mode) {

}

int sys_mknod(const char* path, mode_t mode, dev_t dev) {

}

int sys_mknodat(int dirfd, const char* path, mode_t mode, dev_t dev) {

}

pid_t sys_fork(void) {

}

int sys_execve(const char* pathname, char* const argv[], char* const envp[]) {

}

int sys_execv(const char* pathname, char* const argv[]) {

}

int sys_execvp(const char* file, char* const argv[]) {

}

int sys_execvpe(const char* file, char* const argv[], char* const envp[]) {

}

void sys_exit(int status) {

}

pid_t sys_wait(int* status) {

}

pid_t sys_waitpid(pid_t pid, int* status, int options) {

}

int sys_waitid(int idtype, id_t id, siginfo_t* info, int options) {

}

int sys_kill(pid_t pid, int sig) {

}

pid_t sys_getpid(void) {

}

pid_t sys_getppid(void) {

}

uid_t sys_getuid(void) {

}

uid_t sys_geteuid(void) {

}

gid_t sys_getgid(void) {

}

gid_t sys_getegid(void) {

}

int sys_setuid(uid_t uid) {

}

int sys_seteuid(uid_t euid) {

}

int sys_setgid(gid_t gid) {

}

int sys_setegid(gid_t egid) {

}

int sys_setpgid(pid_t pid, pid_t pgid) {

}

pid_t sys_getpgid(pid_t pid) {

}

pid_t sys_getpgrp(void) {

}

int sys_setpgrp(void) {

}

int sys_setsid(void) {

}

pid_t sys_getsid(pid_t pid) {

}
