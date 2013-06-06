/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description:	Secondary bootloader that permanently resides in sector zero
 * 				flash memory. Uses UART0 and the XMODEM 1K protocol to load
 * 				an new application into sectors 1 onwards. Also redirects
 * 				interrupts to the functions contained in the application
 *
 * Copyright(C) 2010, NXP Semiconductor
 * All rights reserved.
 *
 *****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 *****************************************************************************/
#include <LPC11xx.h>
#include "IAP.h"
#include "crc.h"
#include "uart.h"
#include "upload.h"
#include "byprotocol.h"
#include "24c02.h"
#include "param.h"

uint32_t u32BootLoader_AppPresent(void);
uint32_t u32BootLoader_RunAppPresent(void);
//uint8_t  termId[4] = {0x0,0x0,0x0,0x1};

__align(4) uint8_t tmpBuf[FLASH_PAGE_SIZE];


/*****************************************************************************
 ** Function name:  main
 **
 ** Description:	Bootloader control loop.
 **
 ** Parameters:	    None
 **
 ** Returned value: None
 **
 *****************************************************************************/
__asm void runApp()
{

    ldr r0, =0x2000
             ldr r0, [r0]
             mov sp, r0

             /* Load program counter with application reset vector address, located at
                second word of application area. */
             ldr r0, =0x2004
                      ldr r0, [r0]
                      bx  r0

}



void goApp()
{
    /* Verify if a valid user application is present in the upper sectors
    	 of flash memory. */
    if (u32BootLoader_AppPresent() == 0) //升级备份区，不存在升级文件
    {
        /* Valid application not present, execute bootloader task that will
        	 obtain a new application and program it to flash.. */
			
			//首先检查应用程序区是否有效，有效则跳转到应用程序区，否则等待升级
			
			if(u32BootLoader_RunAppPresent()) //再检查应用程序区是否存在有效的应用程序文件
			{
					runApp(); //存在则立即跳转
			}
      UploadTask();

        /* Above function only returns when new application image has been
        	 successfully programmed into flash. Begin execution of this new
        	 application by resetting the device. */
      NVIC_SystemReset();
    }
    else
    {
        /* Valid application located in the next sector(s) of flash so execute */

        /* Load main stack pointer with application stack pointer initial value,
        	 stored at first location of application area */
        if(copyUploadDataToApp() == 0)
        {
            NVIC_SystemReset();
            return;
        }
        clearUploadMagic();
        runApp();
        /* User application execution should now start and never return here.... */
    }
}

void PWM0_Init(int duty)             //  CT32B0  MAT0:50%,50KHz,MAT1:70%,50KHz
{
    unsigned int tmp_duty = 0;
    LPC_TMR32B1->TCR=0;

    LPC_SYSCON->SYSAHBCLKCTRL|=(1<<10);      //打开定时器模块,只有提供了timer32B0的时钟后才能使用timer

    //LPC_IOCON->PIO0_1  &= ~0x07;
    //LPC_IOCON->PIO0_1  |= 0x03;		/* Timer1_32 MAT0 */
    LPC_IOCON->JTAG_TDO_PIO1_1  &= ~0x07;
    LPC_IOCON->JTAG_TDO_PIO1_1  |= 0x03;		/* Timer0_32 MAT2 */


    LPC_TMR32B1->TCR	= 0x02;                               //定时器复位


    tmp_duty = ( (100-duty) * (SystemAHBFrequency / 1000) ) / 100;

    LPC_TMR32B1->MR0  = tmp_duty;                          //90%占空比

    LPC_TMR32B1->PWMC = 0x01;                       //设置 MA0,1为PWM输出

    LPC_TMR32B1->PR   = 0;                                          //设置分频系数

    LPC_TMR32B1->MR3  = SystemAHBFrequency / 1000;                        //周期控制， 1ms :1K

    LPC_TMR32B1->EMR  = 0x00;                       //01=L,02=H,03=翻转

    LPC_TMR32B1->MCR  = (1<<10);                      //设置如果MR0和TC匹配，TC复位：[2]=1

    LPC_TMR32B1->TCR  =0x01;                           //定时器打开

}
void initParam()
{
    i2c_lpc_init(0);

    loadParam();
		PWM0_Init(brightness);
}
int main(void)
{
    SystemInit();
    initParam();
    UploadTask();
    goApp();
    /* This should never be executed.. */
    return 0;
}


/*****************************************************************************
 *
 *                      Interrupt redirection functions
 *
 *****************************************************************************/
/*****************************************************************************
 ** Function name:   NMI_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
__asm void NMI_Handler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2008
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   HardFault_Handler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void HardFault_Handler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x200C
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   SVCall_Handler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void SVCall_Handler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */

    ldr r0, =0x202C
             ldr r0, [r0]
             bx  r0
             nop

}

         /*****************************************************************************
          ** Function name:   PendSV_Handler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          ******************************************************************************/
         __asm void PendSV_Handler(void)
{

    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2038
             ldr r0, [r0]
             bx  r0
             nop

}

         /*****************************************************************************
          ** Function name:   SysTick_Handler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void SysTick_Handler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x203C
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   WAKEUP_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          ******************************************************************************/
         __asm void WAKEUP_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2040
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   I2C_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void I2C_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x207C
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   TIMER16_0_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void TIMER16_0_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2080
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   TIMER16_1_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void TIMER16_1_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2084
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   TIMER32_0_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void TIMER32_0_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2088
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   TIMER32_1_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void TIMER32_1_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x208C
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   SSP_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void SSP_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2090
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   UART_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void UART_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2094
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   USB_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void USB_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x2098
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   USB_FIQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void USB_FIQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x209C
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   ADC_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void ADC_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20A0
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   WDT_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void WDT_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20A4
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   BOD_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void BOD_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20A8;
    ldr r0, [r0]
    bx  r0
    nop
}

/*****************************************************************************
 ** Function name:   FMC_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
__asm void FMC_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20AC
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   PIOINT3_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void PIOINT3_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20B0
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   PIOINT2_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void PIOINT2_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20B4
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          ** Function name:   PIOINT1_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void PIOINT1_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20B8
             ldr r0, [r0]
             bx  r0
             nop
}



         /*****************************************************************************
          ** Function name:   PIOINT0_IRQHandler
          **
          ** Description:	 Redirects CPU to application defined handler
          **
          ** Parameters:	     None
          **
          ** Returned value:  None
          **
          *****************************************************************************/
         __asm void PIOINT0_IRQHandler(void)
{
    /* Re-direct interrupt, get handler address from application vector table */
    ldr r0, =0x20BC
             ldr r0, [r0]
             bx  r0
             nop
}

         /*****************************************************************************
          **                            End Of File
          *****************************************************************************/
