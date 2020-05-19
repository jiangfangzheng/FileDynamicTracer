# FileDynamicTracer
Linux下基于动态注入技术实现的文件读写追踪器

### 介绍

一款适用于Linux系统的动态追踪文件读写信息的共享库。原理是通过重写与IO相关的函数，在程序运行前优先加载本动态链接库，实现记录文件读写信息。相关的原理可参考我的博客：

https://jfz.me/blog/2020/ld-preload-inject.html

### 使用

导入环境变量指定保存文件的位置和过滤文件路径：

export JFZ_SAVE_FILE=/root/JfzTracer.log

export JFZ_TRACE_PATH=/root/_code/aosp-ninja-trace

在运行想追踪的程序前先LD_PRELOAD注入本动态链接库：

export LD_PRELOAD=/root/_code/FileDynamicTracer/libJfzTracer.so

可以在JfzTracer.log看追踪的文件读写信息了，例如：

```
/root/_code/aosp-ninja-trace/build.ninja
/root/_code/aosp-ninja-trace/build/.ninja_log
/root/_code/aosp-ninja-trace/build/.ninja_deps
/root/_code/aosp-ninja-trace/src/browse.py
/root/_code/aosp-ninja-trace/build/browse_py.h
```

### 风险

本程序复写的函数代码不完美，很有可能导致系统某些基本指令或程序不能正常使用；

**请不要全局载入本动态链接库！**

**请不要用于生产环境！**
