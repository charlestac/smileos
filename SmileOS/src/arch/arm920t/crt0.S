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
** File name:               crt0.S
** Last modified Date:      2012-2-2
** Last Version:            1.0.0
** Descriptions:            �����
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
#include "arch/arm920t/arm.h"
#include "kern/addr_config.h"
/*********************************************************************************************************
** ϵͳ����
*********************************************************************************************************/
#define SYSCALL_BAD         4
#define SYSCALL_NR          100
/*********************************************************************************************************
** �ⲿ���������
*********************************************************************************************************/
.extern cpu_init
.extern irq_c_handler
.extern fiq_c_handler
.extern dabt_c_handler
.extern undf_c_handler
.extern pabt_c_handler
.extern sys_do_table

                            .text
                            .code 32
                            .align 2
/*********************************************************************************************************
** Function name:           _start
** Descriptions:            �쳣������ת��
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
                            .global _start
_start:
    LDR     PC, _reset                      /*  ��λ                                                    */
    LDR     PC, _undf                       /*  δ����ָ���쳣                                          */
    LDR     PC, _swi                        /*  �����ж�                                                */
    LDR     PC, _pabt                       /*  Ԥȡָ��ֹ�쳣                                          */
    LDR     PC, _dabt                       /*  ���ݷ����ж��쳣                                        */
    NOP                                     /*  ����                                                    */
    LDR     PC, _irq                        /*  IRQ �ж�                                                */
    LDR     PC, _fiq                        /*  FIQ �ж�                                                */

_reset:     .word reset_handler
_undf:      .word undef_handler
_swi:       .word swi_handler
_pabt:      .word pabt_handler
_dabt:      .word dabt_handler
_irq:       .word irq_handler
_fiq:       .word fiq_handler
/*********************************************************************************************************
** Function name:           reset_handler
** Descriptions:            ��λ��������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
reset_handler:
                                            /*  ���� SVC ģʽ, ���ж�                                   */
    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)

    MOV     R0, #0                          /*  д PID = 0                                              */
    MCR     p15, 0, R0, c13, c0, 0
    NOP
    NOP
    NOP

    LDR     SP, =(IRQ_STACK_P_BASE)         /*  ���� SVC ģʽ�� SP                                      */

    BL      cpu_init                        /*  ��ʼ�� CPU                                              */

    MOV     R0, #0                          /*  �� BSS �ν�������                                       */
    LDR     R1, =__bss_start
    LDR     R2, =__bss_end

bss_loop:
    CMP     R1, R2
    STRLO   R0, [R1], #4
    BLO     bss_loop

    LDR     PC, =main                       /*  ���� main ����                                          */
/*********************************************************************************************************
** Function name:           swi_handler
** Descriptions:            SWI �����жϴ�������
** input parameters:        R0                  ϵͳ���ò���ָ��
                            R1                  ϵͳ���ú�
** output parameters:       NONE
** Returned value:          ϵͳ���÷���ֵ
*********************************************************************************************************/
swi_handler:
    STMFD   SP!, {R2-R12, LR}               /*  ����Ĵ����ͷ��ص�ַ������� SVC ��ջ                   */

    STMFD   SP, {SP, LR}^                   /*  ���� SYS �� USR ģʽ�� SP �� LR                         */
    NOP
    SUB     SP, SP, #8

    CMP     R1, #SYSCALL_NR                 /*  ϵͳ���ú�ʹ�� R1                                       */

bad_syscall:
    MOVGE   R0, R1
    MOVGE   R1, #SYSCALL_BAD                /*  ���ڻ���� SYSCALL_NR ʱ��Ϊ SYSCALL_BAD                */

    LDR     R2, =sys_do_table               /*  ϵͳ���ô�����ת��                                      */

    LDR     R3, [R2, R1, lsl #2]            /*  �õ�ϵͳ���ô���������ַ                                */

    CMP     R3, #0
    BEQ     bad_syscall                     /*  Ϊ NULL ʱ���� bad_syscall                              */

    LDR     LR, =sys_do_return              /*  ϵͳ���ô������ص�ַ                                    */

    MRS     R2, SPSR
    BIC     R2, R2, #(ARM_MODE_MASK)
    ORR     R2, R2, #(ARM_SVC_MODE)
    MSR     CPSR_c, R2

    MOV     PC, R3

sys_do_return:                              /*  һ����ִ��                                              */
    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)

    LDMFD   SP!, {R2-R3}

    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SYS_MODE)

    MOV     SP, R2                          /*  �ָ� SYS �� USR ģʽ�� SP                               */

    MOV     LR, R3                          /*  �ָ� SYS �� USR ģʽ�� LR                               */

    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)

    LDMFD   SP!, {R2-R12, PC}^              /*  ϵͳ���ô�������, ^ ��ʾ���� SPSR �� CPSR               */
/*********************************************************************************************************
** Function name:           irq_handler
** Descriptions:            IRQ �жϴ�������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
irq_handler:
    SUB     LR, LR, #4                      /*  ���� LR, �� IRQ �жϵĵ�ַ                              */

    LDR     SP, =(IRQ_STACK_V_BASE)         /*  ���� IRQ ģʽ�� SP                                      */

    STMFD   SP!, {R0-R12, LR}               /*  ����Ĵ����� IRQ ��ջ                                   */

    LDR     LR, =irq_c_handler_return       /*  ���ص�ַ                                                */

    LDR     PC, =irq_c_handler              /*  ���� IRQ c ��������                                     */

irq_c_handler_return:                       /*  ��һ����ִ��                                            */
    LDMFD   SP!, {R0-R12, PC}^              /*  IRQ �жϴ�������, ^ ��ʾ���� SPSR �� CPSR               */
/*********************************************************************************************************
** Function name:           fiq_handler
** Descriptions:            FIQ �жϴ�������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
fiq_handler:
    SUB     LR, LR, #4                      /*  ���� LR, �� FIQ �жϵĵ�ַ                              */

    LDR     SP, =(IRQ_STACK_V_BASE)         /*  ���� FIQ ģʽ�� SP                                      */

    STMFD   SP!, {R0-R12, LR}               /*  ����Ĵ����� FIQ ��ջ                                   */

    LDR     LR, =fiq_c_handler_return       /*  ���ص�ַ                                                */

    LDR     PC, =fiq_c_handler              /*  ���� FIQ c ��������                                     */

fiq_c_handler_return:                       /*  ��һ����ִ��                                            */
    LDMFD   SP!, {R0-R12, PC}^              /*  FIQ �жϴ�������, ^ ��ʾ���� SPSR �� CPSR               */
/*********************************************************************************************************
** Function name:           dabt_handler
** Descriptions:            ���ݷ�����ֹ�쳣��������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
dabt_handler:
    SUB     LR, LR, #8                      /*  ���� LR, ���ݷ�����ֹ�쳣�ĵ�ַ                         */

    LDR     SP, =(IRQ_STACK_V_BASE)         /*  ���� ABT ģʽ�� SP                                      */

    STMFD   SP!, {R0-R12, LR}               /*  ����Ĵ����ͷ��ص�ַ�� ABT ��ջ                         */

    STMFD   SP, {SP, LR}^                   /*  ���� SYS �� USR ģʽ�� SP �� LR                         */
    NOP
    SUB     SP, SP, #8

    MRS     R0, SPSR                        /*  ���� SYS �� USR ģʽ�� CPSR                             */
    STMFD   SP!, {R0}

    MOV     R0, SP

    LDR     LR, =dabt_c_handler_return      /*  ���ص�ַ                                                */

    LDR     PC, =dabt_c_handler             /*  ���� DABT c ��������                                    */

dabt_c_handler_return:
    LDMFD   SP!, {R0-R2}
    LDMFD   SP!, {R0-R12, PC}^              /*  DABT �쳣��������, ^ ��ʾ���� SPSR �� CPSR              */
/*********************************************************************************************************
** Function name:           pabt_handler
** Descriptions:            Ԥȡָ��ֹ�쳣��������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
pabt_handler:
    SUB     LR, LR, #4                      /*  ���� LR, Ԥȡָ��ֹ�쳣�ĵ�ַ                           */

    LDR     SP, =(IRQ_STACK_V_BASE)         /*  ���� ABT ģʽ�� SP                                      */

    STMFD   SP!, {R0-R12, LR}               /*  ����Ĵ����ͷ��ص�ַ�� ABT ��ջ                         */

    STMFD   SP, {SP, LR}^                   /*  ���� SYS �� USR ģʽ�� SP �� LR                         */
    NOP
    SUB     SP, SP, #8

    MRS     R0, SPSR                        /*  ���� SYS �� USR ģʽ�� CPSR                             */
    STMFD   SP!, {R0}

    MOV     R0, SP

    LDR     LR, =pabt_c_handler_return      /*  ���ص�ַ                                                */

    LDR     PC, =pabt_c_handler             /*  ���� PABT c ��������                                    */

pabt_c_handler_return:
    LDMFD   SP!, {R0-R2}
    LDMFD   SP!, {R0-R12, PC}^              /*  PABT �쳣��������, ^ ��ʾ���� SPSR �� CPSR              */
/*********************************************************************************************************
** Function name:           undef_handler
** Descriptions:            δ����ָ���쳣��������
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
undef_handler:
    LDR     SP, =(IRQ_STACK_V_BASE)         /*  ���� UNDEF ģʽ�� SP                                    */

    STMFD   SP!, {R0-R12, LR}               /*  ����Ĵ����ͷ��ص�ַ�� UNDEF ��ջ                       */

    STMFD   SP, {SP, LR}^                   /*  ���� SYS �� USR ģʽ�� SP �� LR                         */
    NOP
    SUB     SP, SP, #8

    MRS     R0, SPSR                        /*  ���� SYS �� USR ģʽ�� CPSR                             */
    STMFD   SP!, {R0}

    MOV     R0, SP

    LDR     LR, =undf_c_handler_return      /*  ���ص�ַ                                                */

    LDR     PC, =undf_c_handler             /*  ���� UNDEF c ��������                                   */

undf_c_handler_return:
    LDMFD   SP!, {R0-R2}
    LDMFD   SP!, {R0-R12, PC}^              /*  UNDEF �쳣��������, ^ ��ʾ���� SPSR �� CPSR             */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/