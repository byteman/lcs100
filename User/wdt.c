/******************** (C) COPYRIGHT 2013 Embest Info&Tech Co.,LTD. ************
* 文件名: wdt.c
* 作者  : Wuhan R&D Center, Embest
* 日期  : 01/18/2013
* 描述  : 看门狗定时器相关函数原型的实现.
*******************************************************************************
*******************************************************************************
* 历史:
* 01/18/2013		 : V1.0		   初始版本
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* LPC11xx 外设寄存器 */
#include "LPC11xx.h"
#include "gpio.h"
#include "wdt.h"
#include "clkconfig.h"
volatile uint32_t wdt_counter;

uint8_t Falg_WDT=FALSE;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
  * @函数名：WDT_IRQHandler
  * @描述：看门狗定时器中断例程
  * @参数： 无
  * @返回值：无
  */


/**
  * @函数名：WDTInit
  * @描述：初始化看门狗定时器，配置看门狗中断例程
  * @参数： 无
  * @返回值：无
  */
void WDTInit( void )
{
    LPC_SYSCON->WDTCLKSEL      = 0x01;                                  /* 选择WDT时钟源,系统主时钟      */
    LPC_SYSCON->WDTCLKUEN      = 0x00;
    LPC_SYSCON->WDTCLKUEN      = 0x01;                                  /* 更新使能                     */
    LPC_SYSCON->WDTCLKDIV      = 0x3f;                                  /* WDT时钟分频值为1             */

    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<15);                               /* 打开WDT模块                  */
    LPC_WDT->TC =WDT_FEED_VALUE;	                    	/* 定时时间                     */
    LPC_WDT->MOD=(0X03<<0);                                     /* 使能WDT                      */

    LPC_WDT->FEED = 0xAA;                                        /* 喂狗                         */
    LPC_WDT->FEED = 0x55;

}

/**
  * @函数名：WDTFeed
  * @描述：喂食看门狗以阻止它超时
  * @参数： 无
  * @返回值：无
  */
void WDTFeed( void )
{
    /* 喂食顺序 */
    LPC_WDT->FEED = 0xAA;
    LPC_WDT->FEED = 0x55;
    return;
}

/**
  * @}
  */

/**
  * @}
  */
/************* (C) COPYRIGHT 2010 Wuhan R&D Center, Embest *****文件结束*******/
