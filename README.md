# FileDynamicTracer
Linux下基于动态注入技术实现的文件读写追踪器

### 介绍

一款适用于Linux系统的动态追踪文件读写信息的共享库。原理是通过重写与IO相关的函数，在程序运行前优先加载本动态链接库，实现记录文件读写信息。相关的原理可参考我的博客：

https://jfz.me/blog/2020/ld-preload-inject.html

重写文件列表：

```

open、openat、write、rename、renameat、execve、read、readlink

int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
int creat(const char *pathname, mode_t mode);
int openat(int dirfd, const char *pathname, int flags);
int openat(int dirfd, const char *pathname, int flags, mode_t mode);
ssize_t write(int fd, const void *buf, size_t count);
int rename(const char *old, const char *new);
int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
int execve(const char *pathname, char *const argv[], char *const envp[]);
ssize_t read(int fd, void *buf, size_t count);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

```

### 使用

导入环境变量指定保存文件的位置和过滤文件路径：

export JFZ_SAVE_FILE=/root/JfzTracer.log

export JFZ_TRACE_PATH=/root/_code/aosp-ninja-trace

在运行想追踪的程序前先LD_PRELOAD注入本动态链接库：

export LD_PRELOAD=/root/_code/FileDynamicTracer/libJfzTracer.so

可以在JfzTracer.log看追踪的文件读写信息了，例如：

```
unlinkat():/root/_code/aosp-ninja-trace/build
open()0:/root/_code/aosp-ninja-trace/build.ninja
fopen()ab:/root/_code/aosp-ninja-trace/build/.ninja_log
write():/root/_code/aosp-ninja-trace/_ninja_cmd/1589907313732834.log
open64()0:/root/_code/aosp-ninja-trace/src/browse.py
```

### 风险

本程序复写的函数代码不完美，很有可能导致系统某些基本指令或程序不能正常使用；

**请不要全局载入本动态链接库！**

**请不要用于生产环境！**
