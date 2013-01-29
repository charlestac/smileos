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
** File name:               s3c2440_fb.c
** Last modified Date:      2012-3-15
** Last Version:            1.0.0
** Descriptions:            S3C2440 FrameBuffer 驱动
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JiaoJinXing
** Created date:            2012-3-15
** Version:                 1.0.0
** Descriptions:            创建文件
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
** 扩展阅读:
** http://hi.baidu.com/760159/blog/item/b4c408894605c49ea4c272a3.html
**
*********************************************************************************************************/
#include "kern/kern.h"
#include "vfs/device.h"
#include "vfs/driver.h"
#include "vfs/utils.h"
#include <fcntl.h>
#include "s3c2440.h"
#include "drv/fb.h"
#include "kern/mmu.h"
/*********************************************************************************************************
** LCD 型号配置
*********************************************************************************************************/
#define LCD_N35     (1)
#define LCD_PANEL   LCD_N35
/*********************************************************************************************************
** LCD_N35
*********************************************************************************************************/
#if LCD_PANEL == LCD_N35

#define CLKVAL      (4)                                                 /*  LCD 频率                    */
#define PNRMODE     (3)                                                 /*  显示模式: TFT LCD panel     */
#define BPPMODE     (12)                                                /*  BPP 模式: 16 bpp for TFT    */
#define ENVID       (1)                                                 /*  开启视频输出                */

#define VBPD        (1)                                                 /*  垂直同步信号后肩            */
#define LINEVAL     (240)                                               /*  垂直尺寸                    */
#define VFPD        (5)                                                 /*  垂直同步信号前肩            */
#define VSPW        (1)                                                 /*  垂直同步信号脉宽            */

#define HBPD        (39)                                                /*  水平同步信号后肩            */
#define HOZVAL      (320)                                               /*  水平尺寸                    */
#define HFPD        (14)                                                /*  水平同步信号前肩            */
#define HSPW        (5)                                                 /*  水平同步信号脉宽            */

#define OFFSIZE     (0)                                                 /*  虚拟屏幕偏移大小(单位半字)  */
#define PAGEWIDTH   (HOZVAL)                                            /*  虚拟屏幕宽度(单位半字)      */

#define FRM565      (1)                                                 /*  16 BPP 视频数据格式: RGB565 */
#define PWREN       (1)                                                 /*  使能 LCD_PWREN 输出信号     */
#define ENLEND      (0)                                                 /*  禁能 LEND 输出信号          */

#define BSWP        (0)                                                 /*  字节不交换                  */
#define HWSWP       (1)                                                 /*  半字交换                    */
#define BPP24BL     (0)                                                 /*  24 BPP 视频数据字节序: LSB  */

#define INVVCLK     (0)                                                 /*  不反转 VCLK                 */
#define INVVLINE    (1)                                                 /*  反转 VLINE                  */
#define INVVFRAME   (1)                                                 /*  反转 VFRAME                 */
#define INVVD       (0)                                                 /*  不反转 VD                   */
#define INVVDEN     (0)                                                 /*  不反转 VDEN                 */
#define INVPWREN    (0)                                                 /*  不反转 PWREN                */
#define INVLEND     (0)                                                 /*  不反转 LEND                 */

#define LCD_WIDTH   (HOZVAL)
#define LCD_HEIGHT  (LINEVAL)
#define LCD_BPP     16

#endif
/*********************************************************************************************************
** 全局变量
*********************************************************************************************************/
static uint16_t    *framebuffer;                                        /*  视频帧缓冲                  */

static atomic_t     fb_ref;                                             /*  打开计数                    */
/*********************************************************************************************************
** Function name:           lcd_init
** Descriptions:            初始化 LCD
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int lcd_init(void)
{
    GPGUP   = GPGUP  | (1 << 4);                                        /*  GPG4 关闭上拉电阻           */
    GPGCON  = (GPGCON & ~(0x3 << 8)) | 3 << 8;                          /*  GPG4 -> LCD_PWREN           */
    GPGDAT  = GPGDAT | (1 << 4);                                        /*  打开 LCD 电源               */

    GPCCON  = 0xAAAAAAAA;                                               /*  LCD 功能                    */
    GPDCON  = 0xAAAAAAAA;                                               /*  LCD 功能                    */

    LCDCON1 = (LCDCON1 & ~(0x3FF << 8)) | CLKVAL  << 8;                 /*  LCD 频率                    */
    LCDCON1 = (LCDCON1 & ~(0x3   << 5)) | PNRMODE << 5;                 /*  显示模式                    */
    LCDCON1 = (LCDCON1 & ~(0xF   << 1)) | BPPMODE << 1;                 /*  BPP 模式(每像素点字节数)    */
    LCDCON1 = (LCDCON1 & ~(1)) | 0;                                     /*  关闭视频输出                */

    LCDCON2 = (LCDCON2 & ~(0xFF  << 24)) | VBPD << 24;                  /*  垂直同步信号后肩            */
    LCDCON2 = (LCDCON2 & ~(0x3FF << 14)) | (LINEVAL - 1) << 14;         /*  垂直尺寸                    */
    LCDCON2 = (LCDCON2 & ~(0xFF  <<  6)) | VFPD << 6;                   /*  垂直同步信号前肩            */
    LCDCON2 = (LCDCON2 & ~(0x3F)) | VSPW;                               /*  垂直同步信号脉宽            */

    LCDCON3 = (LCDCON3 & ~(0x7F  << 19)) | HBPD << 19;                  /*  水平同步信号后肩            */
    LCDCON3 = (LCDCON3 & ~(0x7FF <<  8)) | (HOZVAL - 1) << 8;           /*  水平尺寸                    */
    LCDCON3 = (LCDCON3 & ~(0xFF)) | HFPD;                               /*  水平同步信号前肩            */

    LCDCON4 = (LCDCON4 & ~(0xFF)) | HSPW;                               /*  水平同步信号脉宽            */

    LCDCON5 = (LCDCON5 & ~(1 << 12)) | BPP24BL   << 12;                 /*  24 BPP 视频数据字节序       */
    LCDCON5 = (LCDCON5 & ~(1 << 11)) | FRM565    << 11;                 /*  16 BPP 视频数据格式         */
    LCDCON5 = (LCDCON5 & ~(1 << 10)) | INVVCLK   << 10;                 /*  VCLK 有效边沿               */
    LCDCON5 = (LCDCON5 & ~(1 <<  9)) | INVVLINE  <<  9;                 /*  是否反转 VLINE              */
    LCDCON5 = (LCDCON5 & ~(1 <<  8)) | INVVFRAME <<  8;                 /*  是否反转 VFRAME             */
    LCDCON5 = (LCDCON5 & ~(1 <<  7)) | INVVD     <<  7;                 /*  是否反转 VD                 */
    LCDCON5 = (LCDCON5 & ~(1 <<  6)) | INVVDEN   <<  6;                 /*  是否反转 VDEN               */
    LCDCON5 = (LCDCON5 & ~(1 <<  5)) | INVPWREN  <<  5;                 /*  是否反转 PWREN              */
    LCDCON5 = (LCDCON5 & ~(1 <<  4)) | INVLEND   <<  4;                 /*  是否反转 LEND               */
    LCDCON5 = (LCDCON5 & ~(1 <<  3)) | PWREN     <<  3;                 /*  是否使能 LCD_PWREN 输出信号 */
    LCDCON5 = (LCDCON5 & ~(1 <<  2)) | ENLEND    <<  2;                 /*  是否使能 LEND 输出信号      */
    LCDCON5 = (LCDCON5 & ~(1 <<  1)) | BSWP      <<  1;                 /*  字节是否交换                */
    LCDCON5 = (LCDCON5 & ~(1 <<  0)) | HWSWP     <<  0;                 /*  半字是否交换                */

    /* 视频帧缓冲区内存地址高位[30:22]  -> LCDSADDR1[29:21] */
    LCDSADDR1 = (LCDSADDR1 & ~(0x1FF << 21)) | (((uint32_t)framebuffer >> 22) & 0x1FF) << 21;

    /* 视频帧缓冲区内存地址低位[21:1]   -> LCDSADDR1[20:0] */
    LCDSADDR1 = (LCDSADDR1 & ~(0x1FFFFF)) | (((uint32_t)framebuffer >> 1) & 0x1FFFFF);

    /* 视频帧缓冲区的结束地址低位[21:1] -> LCDSADDR2[20:0] */
    LCDSADDR2 = (LCDSADDR2 & ~(0x1FFFFF)) | ((((uint32_t)framebuffer + LINEVAL * HOZVAL * 2) >> 1) & 0x1FFFFF);

    LCDSADDR3 = (LCDSADDR3 & ~(0x7FF << 11)) | OFFSIZE << 11;           /*  虚拟屏幕偏移大小            */
    LCDSADDR3 = (LCDSADDR3 & ~(0x7FF)) | PAGEWIDTH;                     /*  虚拟屏幕宽度                */

    LCDINTMSK = (LCDINTMSK & ~(0x3)) | 0;                               /*  屏蔽中断                    */
    LPCSEL    = (LPCSEL & ~(1)) | 0;                                    /*  禁能 LPC3600/LCC3600 模式   */
    TPAL      = 0x00;                                                   /*  不使用调色板                */

    return 0;
}
/*********************************************************************************************************
** Function name:           fb_open
** Descriptions:            打开 FrameBuffer
** input parameters:        ctx                 私有信息
**                          file                文件结构
**                          oflag               打开标志
**                          mode                模式
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int fb_open(void *ctx, file_t *file, int oflag, mode_t mode)
{
    if (atomic_inc_return(&fb_ref) == 1) {
        lcd_init();
        LCDCON1 = (LCDCON1 & ~(1)) | ENVID;                             /*  开启视频输出                */
    }
    return 0;
}
/*********************************************************************************************************
** Function name:           fb_ioctl
** Descriptions:            控制 FrameBuffer
** input parameters:        ctx                 私有信息
**                          file                文件结构
**                          cmd                 命令
**                          arg                 参数
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int fb_ioctl(void *ctx, file_t *file, int cmd, void *arg)
{
    int ret = 0;
    struct fb_var_screeninfo *var;
    struct fb_fix_screeninfo *fix;

    switch (cmd) {
    case FBIOGET_VSCREENINFO:
        var = va_to_mva(arg);
        var->xoffset        = 0;
        var->yoffset        = 0;
        var->xres           = LCD_WIDTH;
        var->yres           = LCD_HEIGHT;
        var->xres_virtual   = var->xres;
        var->yres_virtual   = var->yres;
        var->bits_per_pixel = LCD_BPP;
        var->red.offset     = 11;
        var->red.length     = 5;
        var->green.offset   = 5;
        var->green.length   = 6;
        var->blue.offset    = 5;
        var->blue.length    = 0;
        break;

    case FBIOGET_FSCREENINFO:
        fix = va_to_mva(arg);
        fix->smem_start     = (void *)FB_MEM_VBASE;
        fix->smem_len       = sizeof(uint16_t) * LCD_WIDTH * LCD_HEIGHT;
        fix->xpanstep       = 0;
        fix->ypanstep       = 0;
        fix->ywrapstep      = 0;
        break;

    default:
        ret = -1;
        break;
    }
    return ret;
}
/*********************************************************************************************************
** Function name:           fb_close
** Descriptions:            关闭 FrameBuffer
** input parameters:        ctx                 私有信息
**                          file                文件结构
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
static int fb_close(void *ctx, file_t *file)
{
    if (atomic_dec_return(&fb_ref) == 0) {
        LCDCON1 = (LCDCON1 & ~(1)) | 0;                                 /*  关闭视频输出                */
    }
    return 0;
}
/*********************************************************************************************************
** Function name:           fb_fstat
** Descriptions:            获得 FrameBuffer 状态
** input parameters:        ctx                 私有信息
**                          file                文件结构
** output parameters:       buf                 状态结构
** Returned value:          0 OR -1
*********************************************************************************************************/
static int fb_fstat(void *ctx, file_t *file, struct stat *buf)
{
    buf->st_size = sizeof(uint16_t) * LCD_WIDTH * LCD_HEIGHT;

    return 0;
}
/*********************************************************************************************************
** FrameBuffer 驱动
*********************************************************************************************************/
driver_t fb_drv = {
        .name  = "fb",
        .open  = fb_open,
        .ioctl = fb_ioctl,
        .close = fb_close,
        .fstat = fb_fstat,
};
/*********************************************************************************************************
** Function name:           fb_init
** Descriptions:            创建 FrameBuffer 设备
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          0 OR -1
*********************************************************************************************************/
int fb_init(void)
{
    driver_install(&fb_drv);

    atomic_set(&fb_ref, 0);

    framebuffer = (uint16_t *)FB_MEM_BASE;

    return device_create("/dev/fb0", "fb", NULL);
}
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/
