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
** File name:               driver.h
** Last modified Date:      2012-3-20
** Last Version:            1.0.0
** Descriptions:            ��������
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-3-20
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
#ifndef DRIVER_H_
#define DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vfs/types.h"
/*********************************************************************************************************
** Function name:           driver_lookup
** Descriptions:            ��������
** input parameters:        name                ������
** output parameters:       NONE
** Returned value:          ����
*********************************************************************************************************/
driver_t *driver_lookup(const char *name);
/*********************************************************************************************************
** Function name:           driver_ref_by_name
** Descriptions:            ͨ��������������
** input parameters:        name                ������
** output parameters:       NONE
** Returned value:          ����
*********************************************************************************************************/
driver_t *driver_ref_by_name(const char *name);
/*********************************************************************************************************
** Function name:           driver_install
** Descriptions:            ��װ����
** input parameters:        drv                 ����
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int driver_install(driver_t *drv);
/*********************************************************************************************************
** Function name:           driver_remove
** Descriptions:            ɾ������
** input parameters:        drv                 ����
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int driver_remove(driver_t *drv);
/*********************************************************************************************************
** Function name:           driver_manager_init
** Descriptions:            ��ʼ����������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int driver_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif                                                                  /*  DRIVER_H_                   */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/