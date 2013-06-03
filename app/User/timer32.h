/*****************************************************************************
 *   timer32.h:  Header file for NXP LPC1xxx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.08.20  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TIMER32_H
#define __TIMER32_H

/* The test is either MAT_OUT or CAP_IN. Default is MAT_OUT. */
#define TIMER_MATCH		0

#define EMC0	4
#define EMC1	6
#define EMC2	8
#define EMC3	10

#define MATCH0	(1<<0)
#define MATCH1	(1<<1)
#define MATCH2	(1<<2)
#define MATCH3	(1<<3)
/* 0 或 1 选择32-位定时器 */
//#define TEST_TIMER_NUM		0
#define TEST_TIMER_NUM2		1

#define TIME_INTERVAL	(SystemAHBFrequency/100 - 1) //10ms
/*
SystemFrequency = 48Mhz, SystemAHBFrequency = 1/1 SystemAHBFrequency, 
10ms = [ (48,000,000 /100)-1 ] = (480000-1)
*/

//extern uint32_t Duty_Frequency;
extern uint32_t  Duty_Time;




void delay32Ms(uint8_t timer_num, uint32_t delayInMs);
void enable_timer32(uint8_t timer_num);
void disable_timer32(uint8_t timer_num);
void reset_timer32(uint8_t timer_num);
void init_timer32(uint8_t timer_num, uint32_t timerInterval);

void PWM0_Init(int duty);
void PWM1_Init(int duty);
#endif /* end __TIMER32_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
