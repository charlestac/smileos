@echo off
setlocal enabledelayedexpansion

set num=0

del sys_symbol.c

echo /********************************************************************************************************* >> sys_symbol.c
echo ** 													>> sys_symbol.c
echo **                                    中国软件开源组织 							>> sys_symbol.c
echo **														>> sys_symbol.c
echo **                                   嵌入式实时操作系统							>> sys_symbol.c
echo **														>> sys_symbol.c
echo **                                       SmileOS(TM)							>> sys_symbol.c
echo **														>> sys_symbol.c
echo **                               Copyright  All Rights Reserved						>> sys_symbol.c
echo **														>> sys_symbol.c
echo **--------------文件信息--------------------------------------------------------------------------------	>> sys_symbol.c
echo **														>> sys_symbol.c
echo ** 文   件   名: sys_symbol.c											>> sys_symbol.c
echo **														>> sys_symbol.c
echo ** 创   建   人: MakeSymbol 工具										>> sys_symbol.c
echo **														>> sys_symbol.c
echo ** 文件创建日期: %date:~3,4% 年 %date:~8,2% 月 %date:~11,2% 日						>> sys_symbol.c
echo **														>> sys_symbol.c
echo ** 描        述: SmileOS 操作系统符号表. (此文件由 MakeSymbol 工具自动生成, 请勿修改)			>> sys_symbol.c
echo *********************************************************************************************************/	>> sys_symbol.c										>> sys_symbol.c
echo #include "kern/func_config.h"                                          >> sys_symbol.c
echo #if CONFIG_MODULE_EN > 0                                        >> sys_symbol.c
echo #include "module/symbol.h"										>> sys_symbol.c
echo.														>> sys_symbol.c
echo #define SYMBOL_TABLE_BEGIN const symbol_t sys_symbol_tbl[] = { 						>> sys_symbol.c
echo.  														>> sys_symbol.c
echo #define SYMBOL_TABLE_END   {0, 0, 0} };									>> sys_symbol.c

echo SYMBOL_TABLE_BEGIN												>> sys_symbol.c

echo SYMBOL_TABLE_END												>> sys_symbol.c
echo #endif                                               >> sys_symbol.c
echo /*********************************************************************************************************	>> sys_symbol.c
echo ** END FILE													>> sys_symbol.c
echo *********************************************************************************************************/	>> sys_symbol.c

del func.txt
del obj.txt
@echo on