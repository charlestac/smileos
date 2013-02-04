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
** File name:               asm.S
** Last modified Date:      2012-2-2
** Last Version:            1.0.0
** Descriptions:            ����ļ�
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
** ��Ա������ task_t �ṹ�е�ƫ����
*********************************************************************************************************/
/*
    int32_t                 pid;
    int32_t                 tid;
    reg_t                   regs[CPU_REGS_SIZE];
*/
#define PID_OFFSET          0
#define TID_OFFSET          4
#define REGS_OFFSET         8
/*********************************************************************************************************
** ARM ������ģʽ����
*********************************************************************************************************/
#undef  ARM_MODE_MASK
#define ARM_MODE_MASK       0xFFFFFFE0
/*********************************************************************************************************
** �ⲿ���������
*********************************************************************************************************/
.extern task_switch_hook

                            .text
                            .code 32
                            .align 2
/*********************************************************************************************************
** Function name:           arch_kernel_enter
** Descriptions:            �����ں�
** input parameters:        R0                  ϵͳ���ò���ָ��
                            R1                  ϵͳ���ú�
** output parameters:       NONE
** Returned value:          ϵͳ���÷���ֵ
*********************************************************************************************************/
                            .global arch_kernel_enter
arch_kernel_enter:
    SWI     0
    MOV     PC, LR
/*********************************************************************************************************
** Function name:           interrupt_disable
** Descriptions:            �����ٽ�����
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          �����ٽ�����ǰ�ĵ�ǰ����״̬�Ĵ���
*********************************************************************************************************/
                            .globl interrupt_disable
interrupt_disable:
    MRS     R0, CPSR

    ORR     R1, R0, #(ARM_FIQ_NO + ARM_IRQ_NO)

    MSR     CPSR_c, R1

    MOV     PC, LR
/*********************************************************************************************************
** Function name:           interrupt_resume
** Descriptions:            �˳��ٽ�����
** input parameters:        reg                 �����ٽ�����ǰ�ĵ�ǰ����״̬�Ĵ���
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
                            .globl interrupt_resume
interrupt_resume:
    MSR     CPSR_c, R0

    MOV     PC, LR
/*********************************************************************************************************
** Function name:           arch_in_kernel
** Descriptions:            �ж��Ƿ����ں���
** input parameters:        NONE
** output parameters:       NONE
** Returned value:          TRUE OR FALSE
*********************************************************************************************************/
                            .global arch_in_kernel
arch_in_kernel:
    MRS     R0, CPSR
    BIC     R0, R0, #ARM_MODE_MASK

    CMP     R0, #ARM_USR_MODE                   /*  �ж��Ƿ��� USR ģʽ��                               */
    BEQ     user_mode

    CMP     R0, #ARM_SYS_MODE                   /*  �ж��Ƿ��� SYS ģʽ��                               */
    BEQ     sys_mode

    MOV     R0, #1                              /*  ���� TRUE                                           */
    MOV     PC, LR

user_mode:
sys_mode:
    MOV     R0, #0                              /*  ���� FALSE                                          */
    MOV     PC, LR
/*********************************************************************************************************
** Function name:           arch_switch_context
** Descriptions:            �����л�
** input parameters:        R0                  ԭ�����������ƿ�
                            R1                  �������������ƿ�
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
                            .global arch_switch_context
arch_switch_context:                            /*  arch_switch_context �������쳣�� SVC ģʽ�µ���     */
    MRS     R2, CPSR
    BIC     R2, R2, #ARM_MODE_MASK

    CMP     R2, #ARM_SVC_MODE                   /*  �ж��Ƿ��� SVC ģʽ��                               */
    BEQ     svc_mode_ctx_save                   /*  ���� SVC ģʽ��������������                         */

    B       trap_mode_ctx_save                  /*  �����쳣ģʽ��������������                          */
/*********************************************************************************************************
** Function name:           trap_mode_ctx_save
** Descriptions:            �쳣ģʽ��������������
** input parameters:        R0                  ԭ�����������ƿ�
                            R1                  �������������ƿ�
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
trap_mode_ctx_save:
/*
              +-------------------------------------------+
              | �쳣ʱ�� PC                               |
              +-------------------------------------------+
              |                                           |
              | R0 - R12                                  |
              |                                           |
              +-------------------------------------------+
              | �쳣ʱ�� LR                               |
              +-------------------------------------------+
              | �쳣ʱ�� SPSR                             |
              | SYS / USR ģʽ�� SPSR, ����Ϊ ARM_SVC_MODE|
              | SVC ģʽ�� SPSR, SPSR ��ģʽΪ SYS / USR  |
              +-------------------------------------------+
              | �쳣ʱ�� SP                               |
              +-------------------------------------------+
              | �쳣ʱ�� CPSR                             |
              +-------------------------------------------+
              | ԭ��ֵ                                    |
              +-------------------------------------------+
*/
    ADD     R0, R0, #REGS_OFFSET                /*  R0 = current->regs                                  */

    ADD     R0, R0, #4                          /*  ���޸� SP_backup                                    */

    MRS     R2, SPSR                            /*  �����쳣ʱ�� CPSR                                   */
    STMIA   R0!, {R2}

    STMIA   R0, {SP}^                           /*  �����쳣ʱ�� SP                                     */
    NOP
    ADD     R0, R0, #4

    BIC     R2, R2, #ARM_MODE_MASK
    CMP     R2, #ARM_SVC_MODE                   /*  �ж��쳣ǰ�Ƿ��� SVC ģʽ��                         */
    BEQ     svc_spsr_save                       /*  �������� SVC ģʽ SPSR                              */

    MOV     R2, #ARM_SVC_MODE                   /*  �쳣ǰ���� SYS �� USR ģʽ, ����һ�� ARM_SVC_MODE   */
    STMIA   R0!, {R2}

    B       lp_save                             /*  ���������쳣ʱ�� LR                                 */

svc_spsr_save:                                  /*  ���� SVC ģʽ SPSR                                  */
    MRS     R3, CPSR
    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)
    MRS     R2, SPSR
    MSR     CPSR_cxsf, R3
    STMIA   R0!, {R2}

lp_save:                                        /*  �����쳣ʱ�� LR                                     */
    STMIA   R0, {LR}^
    NOP
    ADD     R0, R0, #4

    LDR     SP, =(IRQ_STACK_V_BASE - 14 * 4)    /*  SP ָ���쳣����ʱ����ļĴ���                       */
    LDMIA   SP!, {R2-R8}                        /*  ���쳣����ʱ����ļĴ���ת�Ƶ� current->regs ��     */
    STMIA   R0!, {R2-R8}
    LDMIA   SP!, {R2-R8}
    STMIA   R0!, {R2-R8}

    B       arch_switch_context_to              /*  ִ�������л�                                        */
/*********************************************************************************************************
** Function name:           svc_mode_ctx_save
** Descriptions:            SVC ģʽ��������������
** input parameters:        R0                  ԭ�����������ƿ�
                            R1                  �������������ƿ�
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
svc_mode_ctx_save:
/*
              +--------------------------------------+
              | PC                                   |
              +--------------------------------------+
              |                                      |
              | R0 - R12                             |
              |                                      |
              +--------------------------------------+
              | �л�ʱ�� LR                          |
              +--------------------------------------+
              | �л�ʱ�� SPSR                        |
              +--------------------------------------+
              | �л�ʱ�� SP                          |
              +--------------------------------------+
              | �л�ʱ�� CPSR                        |
              +--------------------------------------+
              | ԭ��ֵ                               |
              +--------------------------------------+
*/
    ADD     R0, R0, #REGS_OFFSET                /*  R0 = current->regs                                  */

    ADD     R0, R0, #4                          /*  ���޸� SP_backup                                    */

    MRS     R2, CPSR                            /*  �����л�ʱ�� CPSR                                   */
    STMIA   R0!, {R2}

    STMIA   R0!, {SP}                           /*  �����л�ʱ�� SP                                     */

    MRS     R2, SPSR                            /*  �����л�ʱ�� SPSR                                   */
    STMIA   R0!, {R2}

    STMIA   R0!, {LR}                           /*  �����л�ʱ�� LR                                     */

    STMIA   R0!, {R0-R12, LR}                   /*  �����л�ʱ�ļĴ����� current->regs ��               */

    B       arch_switch_context_to              /*  ִ�������л�                                        */
/*********************************************************************************************************
** Function name:           arch_switch_context_to
** Descriptions:            ֱ���л���������
** input parameters:        R1                  �������������ƿ�
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
                            .global arch_switch_context_to
arch_switch_context_to:

    LDR     R0, [R1, #PID_OFFSET]               /*  ������� PID                                        */
    MOV     R0, R0, LSL #25                     /*  PID �ڼĴ�������� 7 λ                             */
    MCR     p15, 0, R0, c13, c0, 0              /*  д PID, MVA �����仯��!!!                           */
    NOP
    NOP
    NOP

    ADD     R1, R1, #REGS_OFFSET                /*  R1 = task[next].regs                                */

    LDMIA   R1!, {R2-R6}                        /*  R2=SP_backup, R3=CPSR_int, R4=SP_int, R5=SPSR_int   */
                                                /*  R6=LR_int                                           */

    MOV     R0, R5                              /*  ������ܻ�Ҫ�õ� R5, ʹ�� R0 �����Ƚ�               */
    BIC     R0, R0, #ARM_MODE_MASK
    CMP     R0, #ARM_SVC_MODE                   /*  �ж� SPSR_int �е�ģʽ�Ƿ�Ϊ SVC ģʽ               */
    BEQ     sys_usr_mode_ctx_switch             /*  ���� SYS �� USR ģʽ�л�                            */
/*********************************************************************************************************
** Function name:           svc_mode_ctx_switch
** Descriptions:            SVC ģʽ�����������л�
** input parameters:        R1                  ������ļĴ�����
                            R2                  SP_backup
                            R3                  CPSR_int
                            R4                  SP_int
                            R5                  SPSR_int
                            R6                  LR_int
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
svc_mode_ctx_switch:
/*
              +--------------------------------------+
              | PC                                   |
              +--------------------------------------+
              |                                      |
              | R0 - R12                             |
              |                                      |
              +--------------------------------------+
              | �л�ʱ�� LR                          |
              +--------------------------------------+
              | �л�ʱ�� SPSR                        |
              +--------------------------------------+
              | �л�ʱ�� SP                          |
              +--------------------------------------+
              | �л�ʱ�� CPSR                        |
              +--------------------------------------+
              | ԭ��ֵ                               |
              +--------------------------------------+
*/
    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)   /*  ���� SVC ģʽ, ���ж�               */

    MOV     SP, R4                              /*  �ָ��л�ʱ�� SP                                     */

    MSR     SPSR_cxsf, R5                       /*  �ָ��л�ʱ�� SPSR                                   */

    MOV     LR, R6                              /*  �ָ��л�ʱ�� LR                                     */

    MOV     R0, R1

    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_UDF_MODE)   /*  ���� UNDEF ģʽ, ���ж�             */

    MSR     SPSR_cxsf, R3                       /*  �л�ʱ�� CPSR -> UNDEF ģʽ�� SPSR                  */

    LDMIA   R0, {R0-R12, PC}^                   /*  �л����µ�����, SPSR -> CPSR                        */
/*********************************************************************************************************
** Function name:           sys_usr_mode_ctx_switch
** Descriptions:            SYS �� USR ģʽ�����������л�
** input parameters:        R1                  ������Ĵ�����
                            R2                  SP_backup
                            R3                  CPSR_int
                            R4                  SP_int
                            R5                  SPSR_int
                            R6                  LR_int
** output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
sys_usr_mode_ctx_switch:
/*
              +-------------------------------------------+
              | �쳣ʱ�� PC                               |
              +-------------------------------------------+
              |                                           |
              | R0 - R12                                  |
              |                                           |
              +-------------------------------------------+
              | �쳣ʱ�� LR                               |
              +-------------------------------------------+
              | �쳣ʱ�� SPSR                             |
              | SYS / USR ģʽ�� SPSR, ����Ϊ ARM_SVC_MODE|
              | SVC ģʽ�� SPSR, SPSR ��ģʽΪ SYS / USR  |
              +-------------------------------------------+
              | �쳣ʱ�� SP                               |
              +-------------------------------------------+
              | �쳣ʱ�� CPSR                             |
              +-------------------------------------------+
              | ԭ��ֵ                                    |
              +-------------------------------------------+
*/
    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SYS_MODE)   /*  ���� SYS ģʽ, ���ж�               */

    MOV     SP, R4                              /*  �ָ��쳣ʱ�� SP                                     */

    MOV     LR, R6                              /*  �ָ��쳣ʱ�� LR                                     */

    MSR     CPSR_c, #(ARM_FIQ_NO + ARM_IRQ_NO + ARM_SVC_MODE)   /*  ���� SVC ģʽ, ���ж�               */

    MOV     SP, R2                              /*  ���� SVC ģʽ�� SP                                  */

    MSR     SPSR_cxsf, R3                       /*  �쳣ʱ�� CPSR ->  SVC ģʽ�� SPSR                   */

    MOV     R0, R1
    LDMIA   R0, {R0-R12, PC}^                   /*  �л����µ�����, SPSR -> CPSR                        */
/*********************************************************************************************************
** END FILE
*********************************************************************************************************/