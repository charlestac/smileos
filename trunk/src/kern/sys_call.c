/*********************************************************************************************************
**
** Copyright (c) 2011 - 2012  Jiao JinXing <JiaoJinXing1987@gmail.com>
**
** Licensed under the Academic Free License version 2.1
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**--------------------------------------------------------------------------------------------------------
** File name:               sys_call.c
** Last modified Date:      2012-2-22
** Last Version:            1.0.0
** Descriptions:            系统调用
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-2-22
** Version:                 1.0.0
** Descriptions:            创建文件
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             JiaoJinXing
** Modified date:           2012-3-23
** Version:                 1.1.0
** Descriptions:            修改没有保存 R7 的错误
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             JiaoJinXing
** Modified date:           2012-3-25
** Version:                 1.2.0
** Descriptions:            加入是否处于内核模式的判断, 以适合内核模式时的系统调用
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             JiaoJinXing
** Modified date:           2012-3-28
** Version:                 1.3.0
** Descriptions:            按 newlib 需求, 重写了部分系统调用
**
*********************************************************************************************************/
#include "kern/config.h"
#include "kern/types.h"
#include "vfs/vfs.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct stat;
struct tms;

#ifdef SMILEOS_KERNEL
#include "kern/kern.h"
extern sys_do_t sys_do_table[];
//#define debug        kcomplain
#define debug(...)
#else
typedef int (*sys_do_t)();
static sys_do_t sys_do_table[1];
#define in_kernel()     0
#define debug(...)
#include <stdio.h>
#include <unistd.h>
#endif

/*
 * newlib 需要如下的桩函数支持:
 *
    extern int _close_r _PARAMS ((struct _reent *, int));
    extern int _execve_r _PARAMS ((struct _reent *, const char *, char *const *, char *const *));
    extern int _fcntl_r _PARAMS ((struct _reent *, int, int, int));
    extern int _fork_r _PARAMS ((struct _reent *));
    extern int _fstat_r _PARAMS ((struct _reent *, int, struct stat *));
    extern int _getpid_r _PARAMS ((struct _reent *));
    extern int _isatty_r _PARAMS ((struct _reent *, int));
    extern int _kill_r _PARAMS ((struct _reent *, int, int));
    extern int _link_r _PARAMS ((struct _reent *, const char *, const char *));
    extern _off_t _lseek_r _PARAMS ((struct _reent *, int, _off_t, int));
    extern int _mkdir_r _PARAMS ((struct _reent *, const char *, int));
    extern int _open_r _PARAMS ((struct _reent *, const char *, int, int));
    extern _ssize_t _read_r _PARAMS ((struct _reent *, int, void *, size_t));
    extern int _rename_r _PARAMS ((struct _reent *, const char *, const char *));
    extern void *_sbrk_r _PARAMS ((struct _reent *, ptrdiff_t));
    extern int _stat_r _PARAMS ((struct _reent *, const char *, struct stat *));
    extern _CLOCK_T_ _times_r _PARAMS ((struct _reent *, struct tms *));
    extern int _unlink_r _PARAMS ((struct _reent *, const char *));
    extern int _wait_r _PARAMS ((struct _reent *, int *));
    extern _ssize_t _write_r _PARAMS ((struct _reent *, int, const void *, size_t));
 */

/*
 * 系统调用号
 */
#define SYS_CALL_EXIT       0
#define SYS_CALL_SLEEP      1
#define SYS_CALL_YIELD      2
#define SYS_CALL_GETTIME    10
#define SYS_CALL_GETPID     11
#define SYS_CALL_GETREENT   12
#define SYS_CALL_OPEN       20
#define SYS_CALL_READ       21
#define SYS_CALL_WRITE      22
#define SYS_CALL_FCNTL      23
#define SYS_CALL_ISATTY     24
#define SYS_CALL_FSTAT      25
#define SYS_CALL_LSEEK      26
#define SYS_CALL_CLOSE      27
#define SYS_CALL_IOCTL      28
#define SYS_CALL_RENAME     30
#define SYS_CALL_UNLINK     31
#define SYS_CALL_LINK       32
#define SYS_CALL_STAT       33
#define SYS_CALL_MKDIR      34
#define SYS_CALL_OPENDIR    40
#define SYS_CALL_READDIR    41
#define SYS_CALL_SEEKDIR    42
#define SYS_CALL_REWINDDIR  43
#define SYS_CALL_TELLDIR    44
#define SYS_CALL_CLOSEDIR   45
#define SYS_CALL_GETCWD     50
#define SYS_CALL_CHDIR      51
#define SYS_CALL_SOCKET     60
#define SYS_CALL_BIND       61
#define SYS_CALL_ACCEPT     62
#define SYS_CALL_CONNECT    63
#define SYS_CALL_LISTEN     64
#define SYS_CALL_NR         100                                         /*  系统调用数                  */

/*
 * 系统调用模板
 */
int syscall_template(void)
{
    int param1 = 0;
    int param2 = 0;
    int param3 = 0;
    int param4 = 0;
    int ret;
    int syscall = SYS_CALL_EXIT;

    /*
     * 根据 APCS, 前四个参数使用 r0 - r3, 后面的参数使用堆栈
     * 因为切换了处理器模式, sp 也切换了, 所以多于 4 个参数时会有问题
     */
    __asm__ __volatile__("mov    r0,  %0": :"r"(param1));               /*  R0 传递参数 1               */
    __asm__ __volatile__("mov    r1,  %0": :"r"(param2));               /*  R1 传递参数 2               */
    __asm__ __volatile__("mov    r2,  %0": :"r"(param3));               /*  R2 传递参数 3               */
    __asm__ __volatile__("mov    r3,  %0": :"r"(param4));               /*  R3 传递参数 4               */
    __asm__ __volatile__("stmfd  sp!, {r7, lr}");                       /*  保存 R7, LR 到堆栈          */
    __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));              /*  R7 传递系统调用号           */
    __asm__ __volatile__("swi    0");                                   /*  软件中断                    */
    __asm__ __volatile__("ldmfd  sp!, {r7, lr}");                       /*  从堆栈恢复 R7, LR           */
    __asm__ __volatile__("mov    %0,  r0": "=r"(ret));                  /*  R0 传递返回值               */

    return ret;
}

/*
 * _exit
 */
void _exit(int status)
{
    int syscall = SYS_CALL_EXIT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        (sys_do_table[syscall])(status);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(status));
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("b      .");
    }
}

/*
 * sleep_tick
 */
static void sleep_tick(unsigned int ticks)
{
    int syscall = SYS_CALL_SLEEP;

    //debug("%s\n", __func__);
    if (in_kernel()) {
        (sys_do_table[syscall])(ticks);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(ticks));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
    }
}

/*
 * sleep
 */
unsigned sleep(unsigned int seconds)
{
    //debug("%s\n", __func__);
    sleep_tick(TICK_PER_SECOND * seconds);
    return 0;
}

/*
 * usleep
 */
int usleep(useconds_t useconds)
{
    //debug("%s\n", __func__);
    sleep_tick(TICK_PER_SECOND * useconds / 1000000);
    return 0;
}

/*
 * yield
 */
void yield(void)
{
    int syscall = SYS_CALL_YIELD;

    //debug("%s\n", __func__);
    if (in_kernel()) {
        (sys_do_table[syscall])();
    } else {
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
    }
}

/*
 * _gettimeofday_r
 */
int _gettimeofday_r(struct _reent *reent, struct timeval *tv, void *tzp)
{
    int ret;
    int syscall = SYS_CALL_GETTIME;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(tv, tzp);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(tv));
        __asm__ __volatile__("mov    r1,  %0": :"r"(tzp));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _close_r
 */
int _close_r(struct _reent *reent, int fd)
{
    int ret;
    int syscall = SYS_CALL_CLOSE;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * ioctl
 */
int ioctl(int fd, int cmd, void *arg)
{
    int ret;
    int syscall = SYS_CALL_IOCTL;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, cmd, arg);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(cmd));
        __asm__ __volatile__("mov    r2,  %0": :"r"(arg));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _fcntl_r
 */
int _fcntl_r(struct _reent *reent, int fd, int cmd, int arg)
{
    int ret;
    int syscall = SYS_CALL_FCNTL;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, cmd, arg);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(cmd));
        __asm__ __volatile__("mov    r2,  %0": :"r"(arg));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _fstat_r
 */
int _fstat_r(struct _reent *reent, int fd, struct stat *buf)
{
    int ret;
    int syscall = SYS_CALL_FSTAT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, buf);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(buf));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _getpid_r
 */
int _getpid_r(struct _reent *reent)
{
    int ret;
    int syscall = SYS_CALL_GETPID;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])();
    } else {
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _isatty_r
 */
int _isatty_r(struct _reent *reent, int fd)
{
    int ret;
    int syscall = SYS_CALL_ISATTY;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _link_r
 */
int _link_r(struct _reent *reent, const char *path1, const char *path2)
{
    int ret;
    int syscall = SYS_CALL_LINK;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path1, path2);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path1));
        __asm__ __volatile__("mov    r1,  %0": :"r"(path2));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _lseek_r
 */
_off_t _lseek_r(struct _reent *reent, int fd, _off_t offset, int whence)
{
    _off_t ret;
    int syscall = SYS_CALL_LSEEK;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, offset, whence);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(offset));
        __asm__ __volatile__("mov    r2,  %0": :"r"(whence));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _mkdir_r
 */
int _mkdir_r(struct _reent *reent, const char *path, int mode)
{
    int ret;
    int syscall = SYS_CALL_MKDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path, mode);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("mov    r1,  %0": :"r"(mode));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _open_r
 */
int _open_r(struct _reent *reent, const char *path, int oflag, int mode)
{
    int ret;
    int syscall = SYS_CALL_OPEN;

    debug("%s %s by %d\n", __func__, path, getpid());
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path, oflag, mode);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("mov    r1,  %0": :"r"(oflag));
        __asm__ __volatile__("mov    r2,  %0": :"r"(mode));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _read_r
 */
_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t nbytes)
{
    _ssize_t ret;
    int syscall = SYS_CALL_READ;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, buf, nbytes);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(buf));
        __asm__ __volatile__("mov    r2,  %0": :"r"(nbytes));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _write_r
 */
_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t nbytes)
{
    _ssize_t ret;
    int syscall = SYS_CALL_WRITE;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(fd, buf, nbytes);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(fd));
        __asm__ __volatile__("mov    r1,  %0": :"r"(buf));
        __asm__ __volatile__("mov    r2,  %0": :"r"(nbytes));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _rename_r
 */
int _rename_r(struct _reent *reent, const char *old, const char *new)
{
    int ret;
    int syscall = SYS_CALL_RENAME;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(old, new);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(old));
        __asm__ __volatile__("mov    r1,  %0": :"r"(new));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _stat_r
 */
int _stat_r(struct _reent *reent, const char *path, struct stat *buf)
{
    int ret;
    int syscall = SYS_CALL_STAT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path, buf);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("mov    r1,  %0": :"r"(buf));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _unlink_r
 */
int _unlink_r(struct _reent *reent, const char *path)
{
    int ret;
    int syscall = SYS_CALL_UNLINK;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 获得 reent 结构指针
 */
struct _reent *getreent(void)
{
    struct _reent *ret;
    int syscall = SYS_CALL_GETREENT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (struct _reent *)(sys_do_table[syscall])();
    } else {
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * _sbrk_r
 */
void *_sbrk_r(struct _reent *reent, ptrdiff_t incr)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

/*
 * _fork_r
 */
int _fork_r(struct _reent *reent)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

/*
 * _times_r
 */
_CLOCK_T_ _times_r(struct _reent *reent, struct tms *buf)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

/*
 * _wait_r
 */
int _wait_r(struct _reent *reent, int *status)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

/*
 * _execve_r
 */
int _execve_r(struct _reent *reent, const char *path, char *const *argv, char *const *env)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

/*
 * _kill_r
 */
int _kill_r(struct _reent *reent, int pid, int sig)
{
    debug("%s\n", __func__);
    _exit(0);
}

/*
 * 打开目录
 */
DIR *opendir(const char *path)
{
    DIR *ret;
    int syscall = SYS_CALL_OPENDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (DIR *)(sys_do_table[syscall])(path);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 关闭目录
 */
int closedir(DIR *dir)
{
    int ret;
    int syscall = SYS_CALL_CLOSEDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(dir);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(dir));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 读目录项
 */
struct dirent *readdir(DIR *dir)
{
    struct dirent *ret;
    int syscall = SYS_CALL_READDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (struct dirent *)(sys_do_table[syscall])(dir);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(dir));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 重置目录读点
 */
int rewinddir(DIR *dir)
{
    int ret;
    int syscall = SYS_CALL_REWINDDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(dir);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(dir));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 调整目录读点
 */
int seekdir(DIR *dir, long loc)
{
    int ret;
    int syscall = SYS_CALL_SEEKDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(dir, loc);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(dir));
        __asm__ __volatile__("mov    r1,  %0": :"r"(loc));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 获得目录读点
 */
long telldir(DIR *dir)
{
    long ret;
    int syscall = SYS_CALL_TELLDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (long)(sys_do_table[syscall])(dir);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(dir));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 改变当前工作目录
 */
int chdir(const char *path)
{
    int ret;
    int syscall = SYS_CALL_CHDIR;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(path);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(path));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * 获得当前工作目录
 */
char *getcwd(char *buf, size_t size)
{
    char *ret;
    int syscall = SYS_CALL_GETCWD;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (char *)(sys_do_table[syscall])(buf, size);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(buf));
        __asm__ __volatile__("mov    r1,  %0": :"r"(size));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * select
 */
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
#ifdef SMILEOS_KERNEL
    printk("can't call %s()!, kill kthread %s tid=%d abort\n", __func__, current->name, current->tid);

    abort();
#else
    printf("can't call %s()!, kill process %d\n", __func__, getpid());

    abort();
#endif
}

#ifndef SMILEOS_KERNEL
/*
 * socket
 */
int socket(int domain, int type, int protocol)
{
    int ret;
    int syscall = SYS_CALL_SOCKET;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(domain, type, protocol);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(domain));
        __asm__ __volatile__("mov    r1,  %0": :"r"(type));
        __asm__ __volatile__("mov    r2,  %0": :"r"(protocol));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * bind
 */
int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
    int ret;
    int syscall = SYS_CALL_BIND;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(s, name, namelen);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(s));
        __asm__ __volatile__("mov    r1,  %0": :"r"(name));
        __asm__ __volatile__("mov    r2,  %0": :"r"(namelen));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * accept
 */
int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;
    int syscall = SYS_CALL_ACCEPT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(s, addr, addrlen);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(s));
        __asm__ __volatile__("mov    r1,  %0": :"r"(addr));
        __asm__ __volatile__("mov    r2,  %0": :"r"(addrlen));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * connect
 */
int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    int ret;
    int syscall = SYS_CALL_CONNECT;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(s, name, namelen);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(s));
        __asm__ __volatile__("mov    r1,  %0": :"r"(name));
        __asm__ __volatile__("mov    r2,  %0": :"r"(namelen));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}

/*
 * listen
 */
int listen(int s, int backlog)
{
    int ret;
    int syscall = SYS_CALL_LISTEN;

    debug("%s\n", __func__);
    if (in_kernel()) {
        ret = (sys_do_table[syscall])(s, backlog);
    } else {
        __asm__ __volatile__("mov    r0,  %0": :"r"(s));
        __asm__ __volatile__("mov    r1,  %0": :"r"(backlog));
        __asm__ __volatile__("stmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    r7,  %0": :"r"(syscall));
        __asm__ __volatile__("swi    0");
        __asm__ __volatile__("ldmfd  sp!, {r7, lr}");
        __asm__ __volatile__("mov    %0,  r0": "=r"(ret));
    }
    return ret;
}
#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
