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
#define TEST_TIMER_NUM		0
#define TEST_TIMER_NUM2		1

#define TIME_INTERVAL	(SystemAHBFrequency/1000 - 1)
/* depending on the SystemFrequency and SystemAHBFrequency setting,
if SystemFrequency = 60Mhz, SystemAHBFrequency = 1/4 SystemAHBFrequency,
10mSec = 150.000-1 counts */

//extern uint32_t Duty_Frequency;
extern uint32_t  Duty_Time;
extern uint32_t  Duty_Time_Temp;//临时占空比
extern uint32_t Delay_timer_counter;
//extern uint8_t  Flag_Delay;



void delay32Ms(uint8_t timer_num, uint32_t delayInMs);
void enable_timer32(uint8_t timer_num);
void disable_timer32(uint8_t timer_num);
void reset_timer32(uint8_t timer_num);
void init_timer32(uint8_t timer_num, uint32_t timerInterval);
void init_timer32PWM(uint8_t timer_num, uint32_t period, uint8_t match_enable);
void setMatch_timer32PWM (uint8_t timer_num, uint8_t match_nr, uint32_t value);
void init_time(void);

#endif /* end __TIMER32_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
