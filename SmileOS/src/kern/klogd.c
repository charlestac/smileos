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
** File name:               klogd.c
** Last modified Date:      2012-3-25
** Last Version:            1.0.0
** Descriptions:            内核日志
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-3-25
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
#include "kern/kern.h"
#include "kern/ipc.h"
#include "kern/func_config.h"
#include <stdio.h>
#include <stdarg.h>

#if CONFIG_VFS_EN > 0
#include <unistd.h>
#include <fcntl.h>
#endif

/*********************************************************************************************************
** 配置
*********************************************************************************************************/
#define KLOGD_QUEUE_SIZE            (100)
#define KLOGD_THREAD_STACKSIZE      (16 * KB)
#define KLOGD_THREAD_PRIO           (5)
#define KLOGD_LOG_LEVEL             (4)

#if CONFIG_VFS_EN > 0
#define KLOGD_LOG_FILE              "/dev/serial0"
#endif
/*********************************************************************************************************
** 内部变量
*********************************************************************************************************/
static mqueue_t             mqueue;                                     /*  内核日志消息队列            */
static uint8_t              log_level = KLOGD_LOG_LEVEL;                /*  LOG 级别                    */
/*********************************************************************************************************
** 内核日志消息
*********************************************************************************************************/
typedef struct {
    int                     len;
    char                    buf[LINE_MAX];
} msg_t;
/*********************************************************************************************************
** Function name:           klogd
** Descriptions:            内核日志线程
** input parameters:        arg                 参数
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
static void klogd(void *arg)
{
    msg_t *msg;
    int fd = -1;

#if CONFIG_VFS_EN > 0
    fclose(stdout);

    while (fd < 0) {
        fd = open(KLOGD_LOG_FILE, O_WRONLY);
        if (fd < 0) {
            sleep(1);
        }
    }

    stdout  = fdopen(fd,  "w");
#endif

    while (1) {
        if (mqueue_fetch(&mqueue, (void **)&msg, 0) == 0) {

#if CONFIG_VFS_EN > 0
            if (msg->buf[msg->len - 1] == '\n' && msg->buf[msg->len - 2] != '\r') {
                msg->buf[msg->len - 1] = '\r';
                msg->buf[msg->len]     = '\n';
                write(fd, msg->buf, msg->len + 1);
            } else {
                write(fd, msg->buf, msg->len);
            }
#else
            /*
             * TODO: 不依赖于 VFS 的调试通道
             */
#endif
            kfree(msg);
        }
    }
}
/*********************************************************************************************************
** Function name:           klogd_create
** Descriptions:            创建内核日志线程
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void klogd_create(void)
{
    mqueue_create(&mqueue, KLOGD_QUEUE_SIZE);

    kthread_create("klogd", klogd, NULL, KLOGD_THREAD_STACKSIZE, KLOGD_THREAD_PRIO);
}
/*********************************************************************************************************
** Function name:           printk
** Descriptions:            终止内核前的报警也不能使用
** input parameters:        fmt                 格式字符串
**                          ...                 其余参数
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void printk(const char *fmt, ...)
{
    va_list     va;
    msg_t      *msg;
    uint8_t     level;

    if (fmt == NULL) {
        return;
    }

    if (fmt[0] == '<' && fmt[2] == '>') {
        level = fmt[1] - '0';
        if (level >= 0 && level <= 7) {
            if (level > log_level) {
                return;
            }
            fmt += 3;
        }
    }

    msg = kmalloc(sizeof(msg_t), GFP_KERNEL);
    if (msg == NULL) {
        /*
         * TODO: 不依赖于 VFS 的调试通道
         */
        return;
    }

    va_start(va, fmt);

    msg->len = vsnprintf(msg->buf, sizeof(msg->buf), fmt, va);

    va_end(va);

    if (mqueue_post(&mqueue, msg, 0) < 0) {
        kfree(msg);
    }
}
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/
