@echo off
setlocal enabledelayedexpansion

set srcfile=../../../SmileOS/src/runtime/arch/arm920t/libemWin.a

nm %srcfile% | find " T "    >> func.txt
nm %srcfile% | find " D "    >> obj.txt
nm %srcfile% | find " B "    >> obj.txt
nm %srcfile% | find " R "    >> obj.txt
nm %srcfile% | find " S "    >> obj.txt
nm %srcfile% | find " C "    >> obj.txt
nm %srcfile% | find " W "    >> obj.txt
nm %srcfile% | find " V "    >> obj.txt

set num=0

del symbol.c

echo /********************************************************************************************************* >> symbol.c
echo ** 													>> symbol.c
echo **                                    中国软件开源组织 							>> symbol.c
echo **														>> symbol.c
echo **                                   嵌入式实时操作系统							>> symbol.c
echo **														>> symbol.c
echo **                                       SmileOS(TM)							>> symbol.c
echo **														>> symbol.c
echo **                               Copyright  All Rights Reserved						>> symbol.c
echo **														>> symbol.c
echo **--------------文件信息--------------------------------------------------------------------------------	>> symbol.c
echo **														>> symbol.c
echo ** 文   件   名: symbol.c											>> symbol.c
echo **														>> symbol.c
echo ** 创   建   人: MakeSymbol 工具										>> symbol.c
echo **														>> symbol.c
echo ** 文件创建日期: %date:~3,4% 年 %date:~8,2% 月 %date:~11,2% 日						>> symbol.c
echo **														>> symbol.c
echo ** 描        述: emWin 图形库符号表. (此文件由 MakeSymbol 工具自动生成, 请勿修改)			>> symbol.c
echo *********************************************************************************************************/	>> symbol.c										>> symbol.c
echo #include "kern/func_config.h"                                          >> symbol.c
echo #if CONFIG_MODULE_EN > 0                                        >> symbol.c
echo #include "module/symbol_tool.h"										>> symbol.c
echo.														>> symbol.c
echo #define SYMBOL_TABLE_BEGIN symbol_t gui_symbol_tbl[] = { 						>> symbol.c
echo.  														>> symbol.c
echo #define SYMBOL_TABLE_END   {0, 0, 0} };									>> symbol.c
echo.														>> symbol.c
echo #define SYMBOL_ITEM_FUNC(pcName)                \>> symbol.c
echo     {   #pcName, (void *)pcName,                \>> symbol.c
echo         SYMBOL_TEXT                             \>> symbol.c
echo     },													>> symbol.c
echo.														>> symbol.c
echo #define SYMBOL_ITEM_OBJ(pcName)                 \>> symbol.c
echo     {   #pcName, (void *)^&pcName,               \>> symbol.c
echo         SYMBOL_DATA                             \>> symbol.c
echo     },													>> symbol.c
echo.														>> symbol.c
echo /*********************************************************************************************************	>> symbol.c
echo ** 全局对象声明												>> symbol.c
echo *********************************************************************************************************/	>> symbol.c

for /f "tokens=3 delims= " %%i in (func.txt) do @(
        echo extern int  %%i^(^); >> symbol.c
        set /a num+=1
)

for /f "tokens=3 delims= " %%i in (obj.txt) do @(
        echo extern int  %%i; >> symbol.c
        set /a num+=1
)

echo.
echo /*********************************************************************************************************	>> symbol.c
echo ** 系统静态符号表												>> symbol.c
echo *********************************************************************************************************/	>> symbol.c
echo SYMBOL_TABLE_BEGIN												>> symbol.c

for /f "tokens=3 delims= " %%i in (func.txt) do @(
        echo     SYMBOL_ITEM_FUNC^(%%i^) >> symbol.c
)

for /f "tokens=3 delims= " %%i in (obj.txt) do @(
        echo     SYMBOL_ITEM_OBJ^(%%i^) >> symbol.c
)
echo SYMBOL_TABLE_END												>> symbol.c
echo #endif                                               >> symbol.c
echo /*********************************************************************************************************	>> symbol.c
echo ** END FILE													>> symbol.c
echo *********************************************************************************************************/	>> symbol.c

del func.txt
del obj.txt
@echo on