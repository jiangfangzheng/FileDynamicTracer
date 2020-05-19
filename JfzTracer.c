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

#define MAX 0x100
#define DEFAULT_FILTER "/tmp/"

char FILTER[MAX] = "/tmp/";

void getNamePID(int pid, char *pName) {
    int fp = 0;
    char buff[MAX] = {0,};
    snprintf(buff, MAX, "/proc/%d/cmdline", pid);
    fp = hook_open(buff, 0, 0);
    read(fp, (char *) pName, MAX);
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

void PrintLog(char *real_path, char *msg) {
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
                fprintf(fp, "pName:%s pid:%d ppName:%s ppid:%d path:%s\n", __progname, getpid(), pName, getppid(), real_path);
                fclose(fp);
            }
        }
    }
}

int unlinkat(int dirfd, const char *path, int flags) {
    if (hook_unlinkat == NULL) {
        hook_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
    }
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        PrintLog(real_path, "unlinkat()");
    }
    return hook_unlinkat(dirfd, path, flags);
}

int unlink(const char *path) {
    if (hook_unlink == NULL) {
        hook_unlink = dlsym(RTLD_NEXT, "unlink");
    }
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        PrintLog(real_path, "unlink()");
    }
    return hook_unlink(path);
}

int open(const char *path, int flags, mode_t mode) {
    if (hook_open == NULL) {
        hook_open = dlsym(RTLD_NEXT, "open");
    }
    int hook_ret = hook_open(path, flags, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "open()%d", flags);
        PrintLog(real_path, msg);
    }
    return hook_ret;
}

int open64(const char *path, int flags, mode_t mode) {
    if (hook_open64 == NULL) {
        hook_open64 = dlsym(RTLD_NEXT, "open64");
    }
    int hook_ret = hook_open64(path, flags, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "open64()%d", flags);
        PrintLog(real_path, msg);
    }
    return hook_ret;
}

FILE *fopen(const char *path, const char *mode) {
    if (hook_fopen == NULL) {
        hook_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    FILE *hook_ret = hook_fopen(path, mode);
    char *real_path = realpath(path, 0);
    if (real_path) {
        CheckConfig();
        char msg[0xFFF];
        sprintf(msg, "fopen()%s", mode);
        PrintLog(real_path, msg);
    }
    return hook_ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (hook_write == NULL) {
        hook_write = dlsym(RTLD_NEXT, "write");
    }
    ssize_t hook_ret = hook_write(fd, buf, count);

    // fd to filePath
    char proclnk[0xFFF];
    sprintf(proclnk, "/proc/self/fd/%d", fd);
    char filePath[0xFFF];
    readlink(proclnk, filePath, 0xFFF);

    char *real_path = realpath(filePath, 0);
    if (real_path) {
        CheckConfig();
        PrintLog(real_path, "write()");
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
}
