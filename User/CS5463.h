#ifndef __CS5463_H 
#define __CS5463_H

/* 
 * 宏,用于定义SPI_CS的片选引脚
 */
#define SPI0_CS		   (1<< 2)											/* P0.2口为CS5463的片选， 	*/.
#define CS5463_Reset_Pin  (1<< 2)//复位脚
#define CS5463CS_Low()	 GPIOSetValue(PORT0, 2,0)
#define CS5463CS_High()	 GPIOSetValue(PORT0, 2,1)

#define CS5463_Reset_H()	LPC_GPIO1->DATA |= CS5463_Reset_Pin
#define CS5463_Reset_L()	LPC_GPIO1->DATA &=~CS5463_Reset_Pin
/*------------------------------------读取电能数据-----------------------------------------*/
#define WrConfig	0x40	//	写入配置数据

#define WrIgain 0x44	//	写电流增益指令
#define RdIgain 0x04	//	读电流增益指令

#define WrVgain 0x48	//	写电压增益指令
#define RdVgain 0x08	//	读电压增益指令

#define WrCounter	0x4a	//	写一个周期采样点数指令
#define RdCounter	0x0a	//	读一个周期采样点数指令

#define Prms	0x14	//	读有功功率有效值指令
#define Irms	0x16	//	度电流有效值指令
#define Urms	0x18	//	读电压有效值指令

#define WrPgain 0x5c	//	写功率偏移量
#define RdPgain 0x1c	//	读功率偏移量

#define WrStatus	0x5e	//	写状态指令
#define RdStatus	0x1e	//	读状态指令

#define WrIacoff	0x60	//	写电流偏移指令
#define RdIacoff	0x20	//	读电流偏移指令

#define Qavg	0x28		//	读平均无功功率指令

#define PFrms	0x32		//	读功率因数指令
#define Srms	0x36		//	读视在功率指令
#define Frms	0x1a

#define WrVacoff	0x62
#define RdVacoff	0x22

#define WrMode	0x64	//	写操作模式

#define WrMask	0x74	//	写中断屏蔽指令

#define Halt	0xA0	//	启动CS5463或

extern void  SSP_Init (void);
extern uint8_t SPI0_ReceiveByte (void);
extern void SPI0_SendByte (uint8_t data);
extern uint32_t CS5463_ReadData(uint8_t RegAdd);
extern void CS5463_Init(void);
extern void Delay_MS(uint32_t ulTime);
extern void Delay_US(uint32_t ulTime);



#endif

