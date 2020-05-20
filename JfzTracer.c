#define _GNU_SOURCE

#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<string.h>

extern char *__progname;

static int (*hook_open)(const char *path, int flags, mode_t mode) = NULL;

static int (*hook_open64)(const char *path, int flags, mode_t mode) = NULL;

static FILE *(*hook_fopen)(const char *path, const char *mode) = NULL;

static int (*hook_unlinkat)(int dirfd, const char *path, int flags) = NULL;

static int (*hook_unlink)(const char *path) = NULL;

static ssize_t (*hook_write)(int fd, const void *buf, size_t count) = NULL;

static int (*hook_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = NULL;

static int (*hook_rename)(const char *old, const char *new) = NULL;

static int (*hook_renameat)(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) = NULL;

static int (*hook_execve)(const char *pathname, char *const argv[], char *const envp[]) = NULL;

static ssize_t (*hook_read)(int fd, void *buf, size_t count) = NULL;

static ssize_t (*hook_readlink)(const char *pathname, char *buf, size_t bufsiz) = NULL;

static ssize_t (*hook_readlinkat)(int dirfd, const char *pathname, char *buf, size_t bufsiz) = NULL;

#define MAX 0x100
#define DEFAULT_FILTER "/tmp/"

char FILTER[MAX] = "/tmp/";

void getNamePID(int pid, char *pName) {
    int fp = 0;
    char buff[MAX] = {0,};
    snprintf(buff, MAX, "/proc/%d/cmdline", pid);
    if (hook_open == NULL) {
        hook_open = dlsym(RTLD_NEXT, "open");
    }
    fp = hook_open(buff, 0, 0);
    if (hook_read == NULL) {
        hook_read = dlsym(RTLD_NEXT, "read");
    }
    hook_read(fp, (char *) pName, MAX);
    close(fp);
}

void CheckConfig() {
    char *envPathChar = getenv("JFZ_TRACE_PATH");
    if (envPathChar != NULL && strlen(envPathChar) >= 1) {
        strncpy(FILTER, envPathChar, sizeof(FILTER));
    } else {
        strncpy(FILTER, DEFAULT_FILTER, sizeof(FILTER));
    }
}

void printDebugLog(char *msg, const char *path, int num) {
    FILE *fp = NULL;
    if (hook_fopen == NULL) {
        hook_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    fp = hook_fopen("/tmp/JfzTracerDebug.log", "a+");
    if (fp) {
        fprintf(fp, "%s\t%s\t%d\n", msg, path, num);
        fclose(fp);
    }
}

void printLog(char *real_path, char *msg) {
    char pName[MAX] = {0,};
    FILE *fp = NULL;
    if (strstr(real_path, FILTER) > 0) {
        char *envSaveChar = getenv("JFZ_SAVE_FILE");
        if (envSaveChar != NULL) {
            fp = hook_fopen(envSaveChar, "a+");
            if (fp) {
                fprintf(fp, "%s:%s\n", msg, real_path);
                fclose(fp);
            }
        } else {
            fp = hook_fopen("/tmp/JfzTracer.log", "a+");
            if (fp) {
                getNamePID(getppid(), pName);
                fprintf(fp, "pName:%s pid:%d ppName:%s ppid:%d path:%s\n", __progname, getpid(), pName, getppid(),
                        real_path);
                fclose(fp);
            }
        }
    }
}

int unlinkat(int dirfd, const char *path, int flags) {
    printDebugLog("unlinkat()", path, flags);
    if (hook_unlinkat == NULL) {
        hook_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
    }
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "unlinkat()");
    }
    return hook_unlinkat(dirfd, path, flags);
}

int unlink(const char *path) {
    printDebugLog("unlink()", path, -1);
    if (hook_unlink == NULL) {
        hook_unlink = dlsym(RTLD_NEXT, "unlink");
    }
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "unlink()");
    }
    return hook_unlink(path);
}

int open(const char *path, int flags, mode_t mode) {
    printDebugLog("open()", path, flags);
    if (hook_open == NULL) {
        hook_open = dlsym(RTLD_NEXT, "open");
    }
    int hook_ret = hook_open(path, flags, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "open()%d", flags);
        printLog(real_path, msg);
    }
    return hook_ret;
}

int open64(const char *path, int flags, mode_t mode) {
    printDebugLog("open64()", path, flags);
    if (hook_open64 == NULL) {
        hook_open64 = dlsym(RTLD_NEXT, "open64");
    }
    int hook_ret = hook_open64(path, flags, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "open64()%d", flags);
        printLog(real_path, msg);
    }
    return hook_ret;
}

FILE *fopen(const char *path, const char *mode) {
    printDebugLog("open64()", path, -1);
    if (hook_fopen == NULL) {
        hook_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    FILE *hook_ret = hook_fopen(path, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "fopen()%s", mode);
        printLog(real_path, msg);
    }
    return hook_ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    printDebugLog("write()", "fd", fd);
    if (hook_write == NULL) {
        hook_write = dlsym(RTLD_NEXT, "write");
    }
    ssize_t hook_ret = hook_write(fd, buf, count);

    // fd to filePath
    char proclnk[0xFFF];
    sprintf(proclnk, "/proc/self/fd/%d", fd);
    char filePath[0xFFF];
    if (hook_readlink == NULL) {
        hook_readlink = dlsym(RTLD_NEXT, "readlink");
    }
    hook_readlink(proclnk, filePath, 0xFFF);

    char *real_path = realpath(filePath, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "write()");
    }
    return hook_ret;
}

int openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    printDebugLog("openat()", pathname, flags);
    if (hook_openat == NULL) {
        hook_openat = dlsym(RTLD_NEXT, "openat");
    }
    int hook_ret = hook_openat(dirfd, pathname, flags, mode);
    char *real_path = realpath(pathname, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "openat()%d", flags);
        printLog(real_path, msg);
    }
    return hook_ret;
}

int rename(const char *old, const char *new) {
    printDebugLog("rename()", old, -1);
    if (hook_rename == NULL) {
        hook_rename = dlsym(RTLD_NEXT, "rename");
    }
    int hook_ret = hook_rename(old, new);
    char *real_path = realpath(new, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "rename()out");
    }
    return hook_ret;
}

int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) {
    printDebugLog("renameat()", oldpath, -1);
    if (hook_renameat == NULL) {
        hook_renameat = dlsym(RTLD_NEXT, "renameat");
    }
    int hook_ret = hook_renameat(olddirfd, oldpath, newdirfd, newpath);
    char *real_path = realpath(newpath, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "renameat()out");
    }
    return hook_ret;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    printDebugLog("execve()", pathname, -1);
    if (hook_execve == NULL) {
        hook_execve = dlsym(RTLD_NEXT, "execve");
    }
    int hook_ret = hook_execve(pathname, argv, envp);
    char *real_path = realpath(pathname, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "execve()");
    }
    return hook_ret;
}

ssize_t read(int fd, void *buf, size_t count) {
    printDebugLog("read()", "fd", fd);
    if (hook_read == NULL) {
        hook_read = dlsym(RTLD_NEXT, "read");
    }
    ssize_t hook_ret = hook_read(fd, buf, count);

    // fd to filePath
    char proclnk[0xFFF];
    sprintf(proclnk, "/proc/self/fd/%d", fd);
    char filePath[0xFFF];
    if (hook_readlink == NULL) {
        hook_readlink = dlsym(RTLD_NEXT, "readlink");
    }
    hook_readlink(proclnk, filePath, 0xFFF);

    char *real_path = realpath(filePath, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "read()");
    }
    return hook_ret;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    printDebugLog("readlink()", pathname, -1);
    if (hook_readlink == NULL) {
        hook_readlink = dlsym(RTLD_NEXT, "readlink");
    }
    int hook_ret = hook_readlink(pathname, buf, bufsiz);
    char *real_path = realpath(pathname, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "readlink()");
    }
    return hook_ret;
}

ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    printDebugLog("readlinkat()", pathname, -1);
    if (hook_readlinkat == NULL) {
        hook_readlinkat = dlsym(RTLD_NEXT, "readlinkat");
    }
    int hook_ret = hook_readlinkat(dirfd, pathname, buf, bufsiz);
    char *real_path = realpath(pathname, 0);
    if (real_path) {
        CheckConfig();
        printLog(real_path, "readlinkat()");
    }
    return hook_ret;
}

void __attribute__ ((constructor)) before_load(void) {
    if (hook_open == NULL) {
        hook_open = dlsym(RTLD_NEXT, "open");
    }
    if (hook_open64 == NULL) {
        hook_open64 = dlsym(RTLD_NEXT, "open64");
    }
    if (hook_fopen == NULL) {
        hook_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    if (hook_unlink == NULL) {
        hook_unlink = dlsym(RTLD_NEXT, "unlink");
    }
    if (hook_unlinkat == NULL) {
        hook_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
    }
    if (hook_write == NULL) {
        hook_write = dlsym(RTLD_NEXT, "write");
    }
    if (hook_openat == NULL) {
        hook_openat = dlsym(RTLD_NEXT, "openat");
    }
    if (hook_rename == NULL) {
        hook_rename = dlsym(RTLD_NEXT, "rename");
    }
    if (hook_renameat == NULL) {
        hook_renameat = dlsym(RTLD_NEXT, "renameat");
    }
    if (hook_execve == NULL) {
        hook_execve = dlsym(RTLD_NEXT, "execve");
    }
    if (hook_read == NULL) {
        hook_read = dlsym(RTLD_NEXT, "read");
    }
    if (hook_readlink == NULL) {
        hook_readlink = dlsym(RTLD_NEXT, "readlink");
    }
    if (hook_readlinkat == NULL) {
        hook_readlinkat = dlsym(RTLD_NEXT, "readlinkat");
    }
}
