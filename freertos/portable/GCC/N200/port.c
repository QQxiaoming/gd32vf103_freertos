/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM3 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

#include "n200_func.h"
#include "riscv_encoding.h"
#include "n200_timer.h"
#include "n200_eclic.h"

/* Standard Includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/* Each task maintains its own interrupt status in the critical nesting
variable. */
UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

#if USER_MODE_TASKS
	unsigned long MSTATUS_INIT = (MSTATUS_MPIE);
#else
	unsigned long MSTATUS_INIT = (MSTATUS_MPP | MSTATUS_MPIE);
#endif


/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );


/*-----------------------------------------------------------*/

/* System Call Trap */
//ECALL macro stores argument in a2
unsigned long ulSynchTrap(unsigned long mcause, unsigned long sp, unsigned long arg1)	{
	
	switch(mcause&0X00000fff)	{
		//on User and Machine ECALL, handler the request
		case 8:
		case 11:
			if(arg1==IRQ_DISABLE)	{
				//zero out mstatus.mpie
				clear_csr(mstatus,MSTATUS_MPIE);
      
			} else if(arg1==IRQ_ENABLE)	{
				//set mstatus.mpie
				set_csr(mstatus,MSTATUS_MPIE);

			} else if(arg1==PORT_YIELD)		{
				//always yield from machine mode
				//fix up mepc on sync trap
				unsigned long epc = read_csr(mepc);
				vPortYield(sp,epc+4); //切换任务
				
			} else if(arg1==PORT_YIELD_TO_RA)	{
			
				vPortYield(sp,(*(unsigned long*)(sp+1*sizeof(sp)))); //切换任务
			}
			
			break;
		default:
			write(1, "trap\n", 5);
            
                printf("In trap handler, the mcause is %ld\n",(mcause&0X00000fff) );
                printf("In trap handler, the mepc is 0x%lx\n", read_csr(mepc));
                printf("In trap handler, the mtval is 0x%lx\n", read_csr(mbadaddr));
              
			_exit(mcause);
	}

	//fix mepc and return 
	unsigned long epc = read_csr(mepc);

	write_csr(mepc,epc+4);
	return sp;
}


void set_msip_int(void)
{
  *(volatile uint8_t *) (TIMER_CTRL_ADDR + TIMER_MSIP) |=0x01;
}

void clear_msip_int(void)
{
  *(volatile uint8_t *) (TIMER_CTRL_ADDR + TIMER_MSIP) &= ~0x01;
}


unsigned long taskswitch( unsigned long sp, unsigned long arg1)	{
	
	//always yield from machine mode
	//fix up mepc on 
	unsigned long epc = read_csr(mepc);
	vPortYield(sp,epc); //never returns

	return sp;
}
/*-----------------------------------------------------------*/


void vDoTaskSwitchContext(void){
	eclic_set_mth ((configMAX_SYSCALL_INTERRUPT_PRIORITY)|0xf);
	vTaskSwitchContext();
	eclic_set_mth (0);
}
/*-----------------------------------------------------------*/


//进入临界段
void vPortEnterCritical( void )
{
	#if USER_MODE_TASKS
		ECALL(IRQ_DISABLE); //通过ECALL发送关闭中断请求，会在ulSynchTrap中通过改变mpie之后改变mie关闭全局中断
	#else
		eclic_set_mth ((configMAX_SYSCALL_INTERRUPT_PRIORITY)|0xf); //通过eclic的mth寄存器来屏蔽除255之外其他优先级的中断，这个寄存器类似cortex-m里的base_pri
	#endif

	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

//退出临界段
void vPortExitCritical( void )
{
	configASSERT( uxCriticalNesting );
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		#if USER_MODE_TASKS
			ECALL(IRQ_ENABLE);    //通过ECALL发送打开中断请求，会在ulSynchTrap中通过改变mpie之后改变mie开启全局中断
		#else
			eclic_set_mth (0);    //通过eclic的mth寄存器Wi为0解除中断屏蔽
		#endif
	}
	return;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/* Clear current interrupt mask and set given mask */
void vPortClearInterruptMask(int int_mask)
{
	eclic_set_mth (int_mask); 
}
/*-----------------------------------------------------------*/

/* Set interrupt mask and return current interrupt enable register */
int xPortSetInterruptMask()
{
	int int_mask=0;
	int_mask=eclic_get_mth();
	
	eclic_set_mth ((configMAX_SYSCALL_INTERRUPT_PRIORITY)|0xf);
	return int_mask;
}

/*-----------------------------------------------------------*/
/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. */

	//register int *tp asm("x3");
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pxCode;			/* Start address */

	//set the initial mstatus value
	pxTopOfStack--;
	*pxTopOfStack = MSTATUS_INIT;

	pxTopOfStack -= 22;
	*pxTopOfStack = (portSTACK_TYPE)pvParameters;	/* Register a0 */
	//pxTopOfStack -= 7;
	//*pxTopOfStack = (portSTACK_TYPE)tp; /* Register thread pointer */
	//pxTopOfStack -= 2;
	pxTopOfStack -=9;
	*pxTopOfStack = (portSTACK_TYPE)prvTaskExitError; /* Register ra */
	pxTopOfStack--;

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/


void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).
	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	configASSERT( uxCriticalNesting == ~0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/




/* 由于该中断配置为向量模式，则中断到来会调用portasm.S的MTIME_HANDLER,进行栈帧保存之后该函数会调用vPortSysTickHandler*/
void vPortSysTickHandler(){	
	/* 内核timer定时器使用64位的计数器来实现 */
    volatile uint64_t * mtime       = (uint64_t*) (TIMER_CTRL_ADDR + TIMER_MTIME);
    volatile uint64_t * mtimecmp    = (uint64_t*) (TIMER_CTRL_ADDR + TIMER_MTIMECMP);
	
	UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	uint64_t now = *mtime; //当前计数值
    uint64_t then = now + (configRTC_CLOCK_HZ / configTICK_RATE_HZ); //计算下一次tick时间
    *mtimecmp = then;   //写入mtimecmp寄存器

	/* 调用freertos的tick增加接口 */
	if( xTaskIncrementTick() != pdFALSE )
	{
		portYIELD();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}
/*-----------------------------------------------------------*/


void vPortSetupTimer(void)	{
    uint8_t mtime_intattr;
    
	/* 内核timer定时器使用64位的计数器来实现 */
    volatile uint64_t * mtime       = (uint64_t*) (TIMER_CTRL_ADDR + TIMER_MTIME);
    volatile uint64_t * mtimecmp    = (uint64_t*) (TIMER_CTRL_ADDR + TIMER_MTIMECMP);
    uint64_t now = *mtime; //当前计数值
    uint64_t then = now + (configRTC_CLOCK_HZ / configTICK_RATE_HZ); //计算下一次tick时间
    *mtimecmp = then;   //写入mtimecmp寄存器

    mtime_intattr=eclic_get_intattr (CLIC_INT_TMR); //内核timer中断在eclic管理器中clicintattr寄存的地址的值
    mtime_intattr|=ECLIC_INT_ATTR_SHV;              //配置为向量模式
    eclic_set_intattr(CLIC_INT_TMR,mtime_intattr);  //写入寄存器
	
	eclic_irq_enable(CLIC_INT_TMR,configKERNEL_INTERRUPT_PRIORITY>>4,0);  //打开中断 配置优先级为最高（4位优先级组全配置为lvl了）
}

void vPortSetupMSIP(void){
	eclic_set_irq_lvl_abs(CLIC_INT_SFT,1);
	eclic_set_vmode(CLIC_INT_SFT);
    eclic_enable_interrupt (CLIC_INT_SFT);
}
/*-----------------------------------------------------------*/


void vPortSetup()	{

	vPortSetupTimer();
	vPortSetupMSIP();
	uxCriticalNesting = 0;
}
/*-----------------------------------------------------------*/
