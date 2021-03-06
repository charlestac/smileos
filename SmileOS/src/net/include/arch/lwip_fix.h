/*********************************************************************************************************
**
** Copyright (c) 2011 - 2012  Jiao JinXing <jiaojinxing1987@gmail.com>
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
** File name:               lwip_fix.h
** Last modified Date:      2012-2-22
** Last Version:            1.0.0
** Descriptions:            lwIP 修正头文件
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-2-22
** Version:                 1.0.0
** Descriptions:            创建文件
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************/
#ifndef LWIP_FIX_H_
#define LWIP_FIX_H_
/*********************************************************************************************************
** 头文件
*********************************************************************************************************/
#ifdef SMILEOS_KERNEL
#include "kern/kern.h"
#else
#include "kern/types.h"
#endif
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
/*********************************************************************************************************
** 主机与网络字节转换
*********************************************************************************************************/
#define LWIP_PLATFORM_BYTESWAP      0                                   /*  使用 lwIP htonl() ...       */

#if BYTE_ORDER == LITTLE_ENDIAN
static inline u16_t __HTONS(u16_t x)
{
    return (u16_t)((((x) & 0x00FF) << 8) |
                   (((x) & 0xFF00) >> 8));
}
static inline u32_t __HTONL(u32_t x)
{
    return (u32_t)((((x) & 0x000000FF) << 24) |
                   (((x) & 0x0000FF00) <<  8) |
                   (((x) & 0x00FF0000) >>  8) |
                   (((x) & 0xFF000000) >> 24));
}
#define LWIP_PLATFORM_HTONS(x)      __HTONS(x)
#define LWIP_PLATFORM_HTONL(x)      __HTONL(x)
#else
#define LWIP_PLATFORM_HTONS(x)      x
#define LWIP_PLATFORM_HTONL(x)      x
#endif                                                                  /*  BYTE_ORDER == LITTLE_ENDIAN */
/*********************************************************************************************************
** 调试输出
*********************************************************************************************************/
#define U16_F                       "u"
#define U32_F                       "u"
#define S16_F                       "d"
#define S32_F                       "d"
#define X16_F                       "X"
#define X32_F                       "X"
#define SZT_F                       "u"
#define X8_F                        "02X"

#ifdef SMILEOS_KERNEL
#define LWIP_PLATFORM_DIAG(x)       { printk x; }
#define LWIP_PLATFORM_ASSERT(x)     { printk(KERN_ERR"lwip assert: %s\n", x); }
#else
#include <stdio.h>
#define LWIP_PLATFORM_DIAG(x)       { printf x; }
#define LWIP_PLATFORM_ASSERT(x)     { printf("lwip assert: %s\n", x); }
#endif
/*********************************************************************************************************
** 临界区保护
*********************************************************************************************************/
#ifdef SMILEOS_KERNEL
typedef reg_t    sys_prot_t;

#define sys_arch_protect            interrupt_disable
#define sys_arch_unprotect          interrupt_resume
#endif
/*********************************************************************************************************
** OS 数据类型
*********************************************************************************************************/
struct mutex;
struct sem;
struct mqueue;

typedef struct mutex  *             sys_mutex_t;
typedef struct sem    *             sys_sem_t;
typedef struct mqueue *             sys_mbox_t;
typedef int32_t                     sys_thread_t;

#define socklen_t                   int

#define SYS_MUTEX_NULL              0ul
#define SYS_SEM_NULL                0ul
#define SYS_MBOX_NULL               0ul

#define LWIP_COMPAT_MUTEX           0                                   /*  lwIP 不使用兼容的互斥量     */
#define LWIP_TIMEVAL_PRIVATE        0                                   /*  lwIP 不定义 struct timeval  */
/*********************************************************************************************************
** ERRNO 定义
*********************************************************************************************************/
//#define LWIP_PROVIDE_ERRNO        0                                   /*  lwIP 不提供错误代号         */
#define ENSRNOTFOUND                163                                 /*  Domain name not found       */
/*********************************************************************************************************
** 时间性能测量操作宏
*********************************************************************************************************/
#define PERF_START
#define PERF_STOP(x)

#endif                                                                  /*  LWIP_FIX_H_                 */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/
