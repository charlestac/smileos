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
** File name:               symbol.h
** Last modified Date:      2012-7-18
** Last Version:            1.0.0
** Descriptions:            符号
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-7-18
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
#ifndef __SYMBOL_H
#define __SYMBOL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** 类型
*********************************************************************************************************/
#define SYMBOL_DATA     (0)
#define SYMBOL_TEXT     (1)
/*********************************************************************************************************
** 符号
*********************************************************************************************************/
typedef struct {
    char               *name;                                           /*  名字                        */
    void               *addr;                                           /*  地址                        */
    unsigned char       flags;                                          /*  标志                        */
} symbol_t;

#ifdef __cplusplus
}
#endif

#endif                                                                  /*  __SYMBOL_H                  */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/
