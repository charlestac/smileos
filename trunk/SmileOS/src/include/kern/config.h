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
** File name:               config.h
** Last modified Date:      2012-2-2
** Last Version:            1.0.0
** Descriptions:            ����ͷ�ļ�
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-2-2
** Version:                 1.0.0
** Descriptions:            �����ļ�
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************/
#ifndef KERN_CONFIG_H_
#define KERN_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "syslimits.h"

#ifdef S3C2440_BSP
#include "../../bsp/s3c2440/s3c2440_config.h"
#endif

#define KERN_STACK_SIZE             (8192)                              /*  �ں�ջ�ռ��С              */

#define SIG_USR_STACK_SIZE          (8192)                              /*  �ź��û�ջ�ռ��С          */
#define SIG_KERN_STACK_SIZE         (8192)                              /*  �ź��ں�ջ�ռ��С          */

#define PROCESS_NR                  (32)                                /*  ������, ������ 0            */
#define KTHREAD_NR                  (16)                                /*  �ں��߳���                  */
#define TASK_NR                     (PROCESS_NR + KTHREAD_NR)           /*  ������                      */

#define PROCESS_SPACE_SIZE          (32 * MB)                           /*  ���̿ռ��С                */
#define PROCESS_PARAM_SIZE          (PAGE_SIZE)                         /*  ���̲����ռ��С            */
#define PROCESS_STACK_SIZE          (128 * KB)                          /*  ����ջ�ռ��С              */

#ifdef __cplusplus
}
#endif

#endif                                                                  /*  KERN_CONFIG_H_              */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/