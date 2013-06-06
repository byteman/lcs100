/*****************************************************************************
 *   timer32.c:  32-bit Timer C file for NXP LPC1xxx Family Microprocessors
 *
 *   Copyright(C) 2013, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   20013.1.18  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC11xx.h"
#include "timer32.h"
#include "gpio.h"
uint32_t  Duty_Time=100;//占空比100-20

volatile uint32_t timer32_0_counter = 0;
volatile uint32_t timer32_1_counter = 0;
volatile uint32_t timer32_0_capture = 0;
volatile uint32_t timer32_1_capture = 0;
volatile uint32_t timer32_0_period = 0;
volatile uint32_t timer32_1_period = 0;
uint32_t Delay_timer_counter = 0;


/******************************************************************************
** Function name:		CT32B0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**						executes each 10ms @ 60 MHz CPU Clock
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER32_0_IRQHandler(void)//20ms
{
    if ( LPC_TMR32B0->IR & 0x01 )
    {
        LPC_TMR32B0->IR = 1;				/* clear interrupt flag */
        timer32_0_counter++;
    }
    if ( LPC_TMR32B0->IR & (0x1<<4) )
    {
        LPC_TMR32B0->IR = 0x1<<4;			/* clear interrupt flag */
        timer32_0_capture++;
    }
    return;
}

/******************************************************************************
** Function name:		CT32B1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**						executes each 10ms @ 60 MHz CPU Clock
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER32_1_IRQHandler(void)
{
    if ( LPC_TMR32B1->IR & 0x01 )
    {
        LPC_TMR32B1->IR = 1;			/* clear interrupt flag */
        timer32_1_counter++;
    }
    if ( LPC_TMR32B1->IR & (0x1<<4) )
    {
        LPC_TMR32B1->IR = 0x1<<4;			/* clear interrupt flag */
        timer32_1_capture++;
    }
    return;
}

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void enable_timer32(uint8_t timer_num)
{
    if ( timer_num == 0 )
    {
        LPC_TMR32B0->TCR = 1;
    }
    else
    {
        LPC_TMR32B1->TCR = 1;
    }
    return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer32(uint8_t timer_num)
{
    if ( timer_num == 0 )
    {
        LPC_TMR32B0->TCR = 0;
    }
    else
    {
        LPC_TMR32B1->TCR = 0;
    }
    return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer32(uint8_t timer_num)
{
    uint32_t regVal;

    if ( timer_num == 0 )
    {
        regVal = LPC_TMR32B0->TCR;
        regVal |= 0x02;
        LPC_TMR32B0->TCR = regVal;
    }
    else
    {
        regVal = LPC_TMR32B1->TCR;
        regVal |= 0x02;
        LPC_TMR32B1->TCR = regVal;
    }
    return;
}



void PWM1_Init(int duty)             //  CT32B0  MAT0:50%,50KHz,MAT1:70%,50KHz
{
    unsigned int tmp_duty = 0;

    LPC_TMR32B0->TCR=0;		//关闭timer32B0

    LPC_SYSCON->SYSAHBCLKCTRL|=(1<<9);      //打开定时器模块,时钟开启后才能使能

    LPC_TMR32B0->TCR	= 0x02;               //定时器复位,定时器计数器和预分频计数器在PCLK 的下一个上升沿同步复位0

    LPC_IOCON->PIO0_1  = 0x02;		/* Timer0 MAT2 每个定时器的MAT0-MAT3都有一个对应的引脚，PIO0_1对应的是TIMER0的MAT2 */


    tmp_duty = ( (100-duty) * (SystemAHBFrequency / 1000) ) / 100;

    LPC_TMR32B0->MR2  = tmp_duty;     //设置占空比,必须设置MR2

    LPC_TMR32B0->PWMC = (1<<2);     //设置 MAT2,1为PWM输出

    LPC_TMR32B0->PR   = 0;         //设置分频系数

    LPC_TMR32B0->MR3  = SystemAHBFrequency / 1000;                        //周期控制， 100us :1K

    LPC_TMR32B0->EMR  = 0x00;      //01=L,02=H,03=翻转

    LPC_TMR32B0->MCR  = (1<<10);   //设置如果MR3和TC匹配，TC复位

    LPC_TMR32B0->TCR  =0x01;        //定时器打开

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

