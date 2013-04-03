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


uint32_t u32BootLoader_AppPresent(void);
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

		ldr r0, =0x1000
		ldr r0, [r0]
		mov sp, r0

		/* Load program counter with application reset vector address, located at
		   second word of application area. */
		ldr r0, =0x1004
		ldr r0, [r0]
		bx  r0

}
void goApp()
{
	/* Verify if a valid user application is present in the upper sectors
		 of flash memory. */
	if (u32BootLoader_AppPresent() == 0)
	{
		/* Valid application not present, execute bootloader task that will
			 obtain a new application and program it to flash.. */
		upload_task();

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
		runApp();
		/* User application execution should now start and never return here.... */
	}
}

int main(void)
{
	SystemInit();
	
	upload_task();
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
	ldr r0, =0x1008
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
	ldr r0, =0x100C
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
	
	ldr r0, =0x102C
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
	ldr r0, =0x1038
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
	ldr r0, =0x103C
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
	ldr r0, =0x1040
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
	ldr r0, =0x107C
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
	ldr r0, =0x1080
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
	ldr r0, =0x1084
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
	ldr r0, =0x1088
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
	ldr r0, =0x108C
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
	ldr r0, =0x1090
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
	ldr r0, =0x1094
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
	ldr r0, =0x1098
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
	ldr r0, =0x109C
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
	ldr r0, =0x10A0
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
	ldr r0, =0x10A4
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
	ldr r0, =0x10A8;
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
	ldr r0, =0x10AC
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
	ldr r0, =0x10B0
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
	ldr r0, =0x10B4
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
	ldr r0, =0x10B8
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
	 ldr r0, =0x10BC
	 ldr r0, [r0]
	 bx  r0
	 nop
}

/*****************************************************************************
 **                            End Of File
 *****************************************************************************/
