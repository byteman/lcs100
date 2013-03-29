/******************** (C) COPYRIGHT 2010 Embest Info&Tech Co.,LTD. ************
* 文件名: wdt.h
* 作者  : Wuhan R&D Center, Embest
* 日期  : 01/18/2010
* 描述  : 看门狗定时器相关宏定义和函数原型.
*******************************************************************************
*******************************************************************************
* 历史:
* 01/18/2010		 : V1.0		   初始版本
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#ifndef __WDT_H 
#define __WDT_H

#define WDEN		0x00000001
#define WDRESET		0x00000002
#define WDTOF		0x00000004
#define WDINT		0x00000008

#define WDT_FEED_VALUE		0xFFFFFFFF
/* Fsys_ahb 是 48Mhz. */ 
#define TIMER0_FEED_FREQ	((SystemAHBFrequency/4800 - 1))	

extern void WDT_IRQHandler(void);
extern void WDTInit( void );
extern void WDTFeed( void );
extern void WDT_CLK_Setup ( uint32_t clksrc );

extern volatile uint32_t wdt_counter;
extern uint8_t Falg_WDT;

#endif /* end __WDT_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
/************* (C) COPYRIGHT 2010 Wuhan R&D Center, Embest *****文件结束*******/
