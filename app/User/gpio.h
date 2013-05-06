/*****************************************************************************
 *   gpio.h:  Header file for NXP LPC1xxx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __GPIO_H
#define __GPIO_H
#ifndef NULL
#define NULL	((void *)0)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef TRUE
#define TRUE	(1)
#endif


#define DebugTest 1

#define PORT0		0
#define PORT1		1
#define PORT2		2
#define PORT3		3
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define Relay (1ul << 9)//控制继电器
#define Relay_H()	LPC_GPIO1->DATA |= Relay
#define Relay_L()   LPC_GPIO1->DATA &=~Relay

#define McuLed		 (1ul << 5)
#define McuLedOff() LPC_GPIO1->DATA |=	McuLed
#define McuLedOn()	LPC_GPIO1->DATA &= ~ McuLed


#define Zigbee_CFG		 (1ul << 8)
#define Zigbee_CFG_H()   LPC_GPIO1->DATA |=	 Zigbee_CFG
#define Zigbee_CFG_L()	 LPC_GPIO1->DATA &= ~Zigbee_CFG

//*********************************************************************************************************/
void GPIO_IRQHandler(void);
void GPIOInit( void );
void GPIOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir );
void GPIOSetValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal );
void GPIOSetInterrupt( uint32_t portNum, uint32_t bitPosi, uint32_t sense,
                       uint32_t single, uint32_t event );
void GPIOIntEnable( uint32_t portNum, uint32_t bitPosi );
void GPIOIntDisable( uint32_t portNum, uint32_t bitPosi );
uint32_t GPIOIntStatus( uint32_t portNum, uint32_t bitPosi );
void GPIOIntClear( uint32_t portNum, uint32_t bitPosi );

#endif /* end __GPIO_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
