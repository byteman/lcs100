#include "LPC11xx.h"
#include <stdio.h>
#include "CS5463.h"
#include "gpio.h"

/*******************************************************************************
函数名	 ： myDelay
函数原形 ： void myDelay(uint32_t)
功能描述 ： 毫秒级延时
输入参数 ： 无
输出参数 ： 无
返回值	 ： 无
*******************************************************************************/
void Delay_MS(uint32_t ulTime)
{
    uint32_t i = 0;
    while (ulTime--)
    {
        for (i = 0; i < 5000; i++);
    }
}
void Delay_US(uint32_t ulTime)
{
    uint32_t i = 0;
    while (ulTime--)
    {
        for (i = 0; i < 5; i++);
    }
}



/*******************************************************************************
函数名	 ： CS5463_Reset
函数原形 ： void CS5463_Reset(void)
功能描述 ： CS5463硬件复位
说明：PDF上的复位说明：
	  1．使CS脚为低(若CS已为低,使CS为高,再为低)
		 本程序采用硬件SPI操作CS只需向SPI发送无用的数据(0xFF)即可。
	  2．硬件复位(使RESET脚为低,持续最少10uS)
	  3．向串口发初始化序列,该序列包括3个(或更多个)时钟周期的SYNC1命令字(0xFF ),
		 紧跟着一个时钟周期的SYNC0命令字(0xFE)。
输入参数 ： 无
输出参数 ： 无
返回值	 ： 无
*******************************************************************************/
void CS5463_Reset(void)
{
    uint8_t i=0;
    CS5463CS_Low();
    CS5463_Reset_H();
    Delay_US(100);
    CS5463_Reset_L();
    Delay_US(500);
    CS5463_Reset_H();
    CS5463CS_High();
    Delay_US(100);
    CS5463CS_Low();
    Delay_US(100);
    //向SPI总线发送无用数据以在CS上产生下降沿。
    Delay_MS(1);
    for(i=0; i<5; i++)
    {
        SPI0_SendByte(0xFF);
    }
    SPI0_SendByte(0xFE);
}
/*******************************************************************************
函数名	 ： CS5463_Init
函数原形 ： void CS5463_Init(void)
功能描述 ： 初始化CS5463,即配置CS5463的一些基本寄存器
说明：		CS5463上电后，第一步是给复位脉冲到RESET脚，然后在初始化CS5463。
初始化步骤：
  1．在串口上发0x5e命令,再发0x800000数据(即把0x800000写到状态寄存器,用途是清除状态寄存器DRDY位等功能)。
  2．在串口上发0x40命令,再发0x000001数据(即把0x000001写到配置寄存器,用途是设定K=1等功能)。
  3．在串口上发0x4a命令,再发0x000fa0数据(即把0x000fa0写到周期计数寄存器,用途是设定N=4000)。
  4．在串口上发0x74命令,再发0x000000数据(即把0x000000写到屏蔽计数寄存器,用途是不激活INT脚等功能)。
  5．在串口上发0x64命令,再发0x000001数据(即把0x000001写到操作模式寄存器,用途是激活自动在线频率测量等功能)。
  6．把校准得到的DCoff与ACgain的值从E2PROM写到相应的寄存器。
  7．在串口上发0xe8命令(即把采样/测量模式设为连续计算周期）。
接下来就可以去读相应的寄存器，来得到测量值。
输入参数 ： 无
输出参数 ： 无
返回值	 ： 无
*******************************************************************************/
void CS5463_Init(void)
{
    CS5463_Reset();
#if 0
    //写配置寄存器，设定K=1等功能。
    SPI0_SendByte(0x40);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x01);
    Delay_MS(1);
    //写操作模式寄存器，激活自动在线频率测量等功能。
    SPI0_SendByte(0x64);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x61);
    Delay_MS(1);
    //=========================
    SPI0_SendByte(0x42);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x46);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x4C);
    SPI0_SendByte(0x80);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x44);
    SPI0_SendByte(0x20);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x48);
    SPI0_SendByte(0x10);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x60);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x62);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    SPI0_SendByte(0x5c);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    //写状态寄存器，消除状态寄存器DRDY位等功能。
    SPI0_SendByte(0x5e);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x01);
    Delay_MS(1);
    //写屏蔽计数寄存器，不激活INT脚
    SPI0_SendByte(0x74);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(1);
    //写周期计数寄存器，设定N=4000.
    SPI0_SendByte(0x4a);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x01);
    SPI0_SendByte(0x90);
    Delay_MS(1);
#endif
    //写状态寄存器，消除状态寄存器DRDY位等功能。
    SPI0_SendByte(0x5e);
    SPI0_SendByte(0x80);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(10);
    //写配置寄存器，设定K=1等功能。
    SPI0_SendByte(0x40);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x01);
    Delay_MS(10);
    //写周期计数寄存器，设定N=4000.
    SPI0_SendByte(0x4a);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x0F);
    SPI0_SendByte(0xA0);
    Delay_MS(10);
    //写屏蔽计数寄存器，不激活INT脚
    SPI0_SendByte(0x74);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x00);
    Delay_MS(10);
    //写操作模式寄存器，激活自动在线频率测量等功能。
    SPI0_SendByte(0x64);
    SPI0_SendByte(0x80);
    SPI0_SendByte(0x00);
    SPI0_SendByte(0x01);
    Delay_MS(10);
    //校验
    //SPI0_SendByte(0xa0);
    //SPI0_SendByte(0xd6);
    //启动转换
    SPI0_SendByte(0xe8);
    Delay_MS(100);
}
/*******************************************************************************
函数名	 ： CS5463_ReadData
函数原形 ： uint32_t CS5463_ReadData(uint8_t)
功能描述 ： 从指定的寄存器地址读取其内容。
输入参数 ： RegAdd：寄存器地址。
输出参数 ： 无
返回值	 ： 无
*******************************************************************************/
uint32_t CS5463_ReadData(uint8_t RegAdd)
{
    uint8_t data1,data2,data3;
    uint32_t value;
    SPI0_SendByte(RegAdd);//Delay_US(1);
    data1=SPI0_ReceiveByte();//Delay_US(1);
    data2=SPI0_ReceiveByte();//Delay_US(1);
    data3=SPI0_ReceiveByte();//Delay_US(1);
    value=(data1<<16)+(data2<<8)+data3;
    return(value);
}

uint32_t Measuring_220V(void)//测量电压
{
    uint32_t Temp_220V=0;
//	uint32_t CRC16_value=0;
    uint32_t value=0;
    uint32_t AC220V_value=0;
    uint8_t i=0;
    CS5463_ReadData(0x18);
    Delay_MS(10);
    CS5463_ReadData(0x18);
    Delay_MS(10);
    for(i=0; i<10; i++)
    {
        Temp_220V=CS5463_ReadData(0x18);
        if(Temp_220V<6000000)
        {
            AC220V_value=(Temp_220V*150)/6000000;
        }
        else
        {
            AC220V_value=(Temp_220V*200)/7600000;
        }
        value+=AC220V_value;
        Delay_MS(10);
    }
    value=value/10;
    return(value);
}
uint32_t Measuring_AC(void)//测量电流
{
    uint32_t Temp_220V=0;
//		uint32_t CRC16_value=0;
    uint32_t AC220V_value=0;
    uint32_t value=0;
    uint8_t i=0;

    CS5463_ReadData(0x16);
    Delay_MS(10);
    CS5463_ReadData(0x16);
    Delay_MS(10);
    for(i=0; i<10; i++)
    {
        Temp_220V=CS5463_ReadData(0x16);

        if(Temp_220V<270000)
        {
            AC220V_value=(Temp_220V*100)/300000;
        }
        else if(Temp_220V<=290000&&Temp_220V>=270000)
        {
            AC220V_value=(Temp_220V*200)/300000;
        }
        else if(Temp_220V<=330000&&Temp_220V>290000)
        {
            AC220V_value=(Temp_220V*300)/330000;
        }
        else if(Temp_220V<=360000&&Temp_220V>330000)
        {
            AC220V_value=(Temp_220V*400)/380000;
        }
        else
        {
            AC220V_value=(Temp_220V*500)/430000;
        }
        value+=AC220V_value;
        Delay_MS(10);
    }
    value=value/10;
    return(value);

}
uint32_t Measuring_Pactive(void) //测量有功功率
{
    uint32_t Temp_220V=0;
//	uint32_t CRC16_value=0;
    uint32_t value=0;
    uint32_t AC220V_value=0;
    uint8_t i=0;
    CS5463_ReadData(0x14);
    Delay_MS(10);
    CS5463_ReadData(0x14);
    Delay_MS(10);
    for(i=0; i<10; i++)
    {
        Temp_220V=CS5463_ReadData(0x14);
        Temp_220V=(~Temp_220V&0xFFFFFF)+1;
        if(Temp_220V<6000)
        {
            AC220V_value=(Temp_220V*100)/47000;
        }
        else
        {
            AC220V_value=((Temp_220V-6000)*1000)/60000+30;
        }
        value+=AC220V_value;
        Delay_MS(10);
    }
    value=value/10;
    return(value);
}

uint32_t  Light_Measuring_220AV(void)//查询灯状态
{
    uint32_t Temp_220V=0;
//	uint32_t CRC16_value=0;
    uint32_t value=0;
    uint32_t AC220V_value=0;
    uint8_t i=0;
    CS5463_ReadData(0x14);
    Delay_MS(10);
    CS5463_ReadData(0x14);
    Delay_MS(10);
    for(i=0; i<10; i++)
    {
        Temp_220V=CS5463_ReadData(0x14);
        Temp_220V=(~Temp_220V&0xFFFFFF)+1;
        if(Temp_220V<6000)
        {
            AC220V_value=(Temp_220V*100)/47000;
        }
        else
        {
            AC220V_value=((Temp_220V-6000)*1000)/60000+30;
        }
        value+=AC220V_value;
        Delay_MS(10);
    }
    value=value/10;
    return(value);
}



