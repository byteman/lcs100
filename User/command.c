/************************重庆星河光电********************************/

#include "LPC11xx.h"
#include "uart.h"
#include "timer32.h"
#include "24c02.h"
#include <stdio.h>
#include "string.h"
#include "Application.h"
#include "CRC16_1.h"
#include "wdt.h"
#include "math.h"
#include "gpio.h"
uint8_t Data_Buf[Data_Len];
uint8_t field[10];
uint8_t value[10]= {0X64,0X64,0X64,0X64,0X64,0X64,0X64,0X64,0X64,0X64};
uint32_t pwmtimer=10;
uint8_t zigbeeflag=0;
extern volatile  uint32_t timer32_0_counter;
void App_Command(void)//各个命令分解
{
    uint32_t value=0;
    uint8_t Command;
    uint32_t CRC16_value=0;
    uint8_t Light_Status=0x30;
    uint8_t temp_buf[20];
    Command=Data_Buf[7];
    Command=Command&0x7F;
    if(Command==0x07)//调光命令
    {

        Tune_Light();
        Frame_OK(0x07);
    }
    else if(Command==0x33)//测电压
    {
        value=Measuring_220V();
        memset(temp_buf,0,20);
        temp_buf[0]=0x7e;
        temp_buf[1]=0x02;
        memcpy(&temp_buf[2],Terminal_ID,4);
        temp_buf[6]=group_number;
        temp_buf[7]=0x00;      //应答码
        temp_buf[8]=0xF3;     //控制码
        temp_buf[9]=(0xc0+((value&0xFF00)>>8));   //单位为1V      数据
        temp_buf[10]=value&0x00FF;
        CRC16_value=CRC16_1(temp_buf,9);
        temp_buf[11]=((CRC16_value&0xff00)>>8);
        temp_buf[12]=CRC16_value&0x00ff;
        temp_buf[13]=0x0d;
        temp_buf[14]=0x0a;
        UARTSend( (uint8_t *)temp_buf, 15 );
    }
    else if(Command==0x34)//测电流
    {

        value=Measuring_AC();
        memset(temp_buf,0,20);
        temp_buf[0]=0x7e;
        temp_buf[1]=0x02;
        memcpy(&temp_buf[2],Terminal_ID,4);
        temp_buf[6]=group_number;
        temp_buf[7]=0x00;
        temp_buf[8]=0xF4;
        temp_buf[9]=(value&0xFF00)>>8;   //单位为1MA
        temp_buf[10]=value&0x00FF;
        CRC16_value=CRC16_1(temp_buf,9);
        temp_buf[11]=((CRC16_value&0xff00)>>8);
        temp_buf[12]=CRC16_value&0x00ff;
        temp_buf[13]=0x0d;
        temp_buf[14]=0x0a;
        UARTSend( (uint8_t *)temp_buf, 15);
    }
    else if(Command==0x35)//测有功功率
    {
        value= Measuring_Pactive();
        memset(temp_buf,0,20);
        temp_buf[0]=0x7e;
        temp_buf[1]=0x02;
        memcpy(&temp_buf[2],Terminal_ID,4);
        temp_buf[6]=group_number;
        temp_buf[7]=0x00;              //应答码
        temp_buf[8]=0xF5;
        temp_buf[9]=(0x80+((value&0xFF00)>>8));//单位为100MW
        temp_buf[10]=value&0x00FF;
        CRC16_value=CRC16_1(temp_buf,9);
        temp_buf[11]=((CRC16_value&0xff00)>>8);
        temp_buf[12]=CRC16_value&0x00ff;
        temp_buf[13]=0x0d;
        temp_buf[14]=0x0a;
        UARTSend( (uint8_t *)temp_buf, 15 );

    }
    else if(Command==0x01)//zigbee模块配置
    {
        //         Frame_OK(0x01);
        ZigbeeSetup();
        Frame_OK(0x01);

    }
    else if(Command==0x02)//重启
    {

        if(Command==0x02)
        {
            Frame_OK(0x02);
        }
// 		Delay1_MS(100000);
        //	Falg_WDT=TRUE;
    }
    else if(Command==0x12)//修改设备ID
    {
        Modify_ID();
        Frame_OK(0x12);
    }
    else if(Command==0x31)//查询ID
    {
        Inquiry_ID();
    }
    else if(Command==0x0B)//设置默认调光值
    {

        SetupPwm();
        Frame_OK(0x0B);
    }
    else if(Command==0x0C)//设置组号
    {

        SetupGroupNumber();
        Frame_OK(0x0C);
    }
    else if(Command==0x0D)//将当前LED从组中删除
    {

        deleteSetupGroupNumber();
        Frame_OK(0x0D);
    }
    else if(Command==0x39)//查询组号
    {

        InquiryGroupNumber();
    }
    else if(Command==0x36)//查询当前调光值
    {

        InquiryLightValue();
    }
    else if(Command==0x37)//查询默认调光值
    {

        InquiryAcquiesceLightValue();
    }
    else if(Command==0x3C)//查询固件版本号
    {

        Inquiry_Version();
    }
    else if(Command==0x3B)//查询灯状态
    {

        //uint32_t Light_Measuring_220AV();
        if(value<20)//终端状态
        {
            Light_Status=0x31;
        }
        else if(value<10)
        {
            Light_Status=0x32;
        }
        else
        {
            Light_Status=0x30;
        }

    }
    else if(Command==0x0A)//设置调光时间
    {

        Setup_lighttime();
        Frame_OK(0x0A);
    }
    else  if(Command==0x08)//闪烁
    {

        flicker();
        Frame_OK(0x08);
    }
    else  if(Command==0x0E)//设置某场景的调光值
    {

        Setup_fieldlight();
        Frame_OK(0x0E);
    }
    else   if(Command==0x0F)//将某场景值移除
    {

        remove_fieldlight();
        Frame_OK(0x0F);
    }
    else   if(Command==0x11)//进入某种场景
    {

        in_field();
        Frame_OK(0x11);
    }
    else   if(Command==0x3A)//查询灯某场景的调光值
    {

        Inquiry_fieldlightvalue();
    }
    else   if(Command==0x32)//查询所有数据
    {

        Inquiry_alldata();
    }
    else   if(Command==0x38)//查询调光时间
    {

        Inquiry_lighttime();
    }
    else   if(Command==0x06)   //烧写配置信息到EEPROM
    {

        configuration();
    }
    else   if(Command==0x10)//设置通讯波特率
    {

        Baud_Rate ();
        Frame_OK(0x10);
    }
    else   if(Command==0x09)//心跳包
    {

        heartbeat ();
    }
    else   if(Command==0x03)//固件升级请求命令
    {

        upgrade();
    }
    else   if(Command==0x04)//固件升级数据包
    {

        upgradedata();
    }
    else if(Command==0x05)//固件升级校验包
    {

        upgradecrc();
    }
}

/*************************调光函数**********************************************/
void Tune_Light(void)
{

    uint32_t timer=2,pwmtimer1=0,pwmtimer0=0,pwmtimer2=0;
    uint32_t lightvalue=0;

    Duty_Time_Temp=Data_Buf[8];
    lightvalue=Duty_Time;
    if(Duty_Time_Temp!=lightvalue)
    {
        timer = (uint32_t)pow(2,pwmtimer);
        pwmtimer2=(timer*20);
        if(Duty_Time_Temp>lightvalue)
        {
            pwmtimer1 = pwmtimer2/(Duty_Time_Temp-lightvalue);
            if(lightvalue<5)
            {
                lightvalue=5;
                Duty_Time=lightvalue;
                init_timer32PWM(1,TIME_INTERVAL,0x01);
            }
            while((Duty_Time_Temp>lightvalue)&&(lightvalue>=5))
            {

                Delay1_MS(pwmtimer1);
                lightvalue++;
                Duty_Time=lightvalue;
                init_timer32PWM(1,TIME_INTERVAL,0x01);
            }
        }


        if(Duty_Time_Temp<lightvalue)
        {
            pwmtimer0=pwmtimer2/(lightvalue-Duty_Time_Temp);
            while(Duty_Time_Temp<lightvalue)
            {
                if(lightvalue>5)
                {
                    Delay1_MS(pwmtimer0);
                    lightvalue--;
                    Duty_Time=lightvalue;
                    init_timer32PWM(1,TIME_INTERVAL,0x01);
                }
                else
                {
                    lightvalue=0;
                    Duty_Time=lightvalue;
                    init_timer32PWM(1,TIME_INTERVAL,0x01);
                }
                Delay1_MS(1);
            }
        }
    }
    else
    {
        lightvalue=Duty_Time_Temp;
        Duty_Time=lightvalue;
        init_timer32PWM(1,TIME_INTERVAL,0x01);
    }

}
void SetupPwm(void)//设置默认调光值
{
    Duty_Time=Data_Buf[8];

    memset(Data_Buf,0,Data_Len);
    Data_Buf[0]=Duty_Time;
    m24xx_write(EEPROM_24XX02,15,0,Data_Buf,1);	  //向地址0x0f处开始写入1个数字
    memset(Data_Buf,0,Data_Len);
}

void Setup_fieldlight(void)  //设置某场景的调光值
{
    uint8_t num=0;

    num=Data_Buf[8];
    field[num]=num;         //场景号
    value[num]=Data_Buf[9];   //调光值
    m24xx_write(EEPROM_24XX02,50+num,0,&field[num],1);
    m24xx_write(EEPROM_24XX02,60+num,0,&value[num],1);

}
void remove_fieldlight(void)  //将某场景值移除
{
    uint8_t num=0;
    num=Data_Buf[8];
    field[num]=0x00;
    value[num]=0x64;
    m24xx_write(EEPROM_24XX02,50+num,0,&field[num],1);
    m24xx_write(EEPROM_24XX02,60+num,0,&value[num],1);

}
void in_field(void)  //进入某种场景
{
    uint8_t num=0;
    num=Data_Buf[8];     //场景号
    field[num]=num;
    value[num]=value[num];
    Duty_Time=value[num];
    init_timer32PWM(1,TIME_INTERVAL,0x01);
    memset(Data_Buf,0,Data_Len);
    Data_Buf[0]=Duty_Time;
    m24xx_write(EEPROM_24XX02,15,0,Data_Buf,1);	  //向地址0x0f处开始写入1个数字
    memset(Data_Buf,0,Data_Len);
    m24xx_write(EEPROM_24XX02,50+num,0,&field[num],1);

}
void Inquiry_fieldlightvalue(void)  //查询灯某场景的调光值
{
    uint8_t num=0;
    uint8_t temp_buf[20];
    uint32_t CRC16_value=0;
    num=Data_Buf[8];
    value[num]=value[num];
    temp_buf[0]=0x7e;
    temp_buf[1]=0x02;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xFA;    //回复控制码0XC0+0X3A
    temp_buf[9]=num;
    temp_buf[10]=value[num];

    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[11]=((CRC16_value&0xff00)>>8);
    temp_buf[12]=CRC16_value&0x00ff;
    temp_buf[13]=0x0d;
    temp_buf[14]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 15 );
}
void flicker(void)  //闪烁
{
    uint32_t timer=100;
    uint8_t  a=0,num;
    a=Data_Buf[1];         //消息长度
    if(a>0x00)
    {
        for(num=0; num<2; num++)
        {
            timer+=Data_Buf[8+num];
        }
    }

    while(1)
    {
        Duty_Time=100;
        init_timer32PWM(1,TIME_INTERVAL,0x01);
        Delay1_MS(timer);
        Duty_Time=00;
        init_timer32PWM(1,TIME_INTERVAL,0x01);
        Delay1_MS(timer);
    }
}

void SetupGroupNumber(void)//设置组号
{
    group_number=Data_Buf[8];
    memset(Data_Buf,0,Data_Len);
    Data_Buf[0]=group_number;
    m24xx_write(EEPROM_24XX02,17,0,Data_Buf,1); //向地址0x11处开始写入1个数字
    memset(Data_Buf,0,Data_Len);
}

void InquiryGroupNumber(void)//查询组号
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x01;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF9;
    temp_buf[9]=group_number;

    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[10]=((CRC16_value&0xff00)>>8);
    temp_buf[11]=CRC16_value&0x00ff;
    temp_buf[12]=0x0d;
    temp_buf[13]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 14 );
}
void deleteSetupGroupNumber(void)//将当前LED从组中删除
{
    group_number = 0XFF;
    m24xx_write(EEPROM_24XX02,17,0,&group_number,1); //向地址0x11处开始写入1个数字
}

void Inquiry_ID(void)//查询ID号
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x04;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF1;
    memcpy(&temp_buf[9],Terminal_ID,4);
    CRC16_value=CRC16_1(temp_buf,12);
    temp_buf[13]=((CRC16_value&0xff00)>>8);
    temp_buf[14]=CRC16_value&0x00ff;
    temp_buf[15]=0x0d;
    temp_buf[16]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 17 );
}
void Modify_ID(void)    //修改设备ID号
{
    uint8_t  DataL = 0;
    uint8_t  num = 0;
    DataL = Data_Buf[1];      //数据长度
    for(num = 0 ; num < DataL ; num++)
        Terminal_ID[num] = Data_Buf[8+num];
    m24xx_write(EEPROM_24XX02, 10, 0, Terminal_ID, 4);
}
void Setup_lighttime(void)//设置调光时间
{
    pwmtimer=Data_Buf[8];
    Data_Buf[0]=pwmtimer;
    m24xx_write(EEPROM_24XX02,70,0,Data_Buf,1); //向地址0x11处开始写入1个数字
    memset(Data_Buf,0,Data_Len);

}
void Inquiry_Version(void)//查询固件版本号
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x03;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xFC;
    memcpy(&temp_buf[9],Version,3);
    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[12]=((CRC16_value&0xff00)>>8);
    temp_buf[13]=CRC16_value&0x00ff;
    temp_buf[14]=0x0d;
    temp_buf[15]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 16 );
}



void Inquiry_alldata(void)   //查询所有数据
{
    uint8_t temp_buf[25];
    uint32_t ALLdata[11];
    uint8_t  data[1]= {0};
    uint32_t CRC16_value=0;
    ALLdata[0] = Measuring_220V();
    ALLdata[1] = Measuring_AC();
    ALLdata[2] = Measuring_Pactive();
    ALLdata[3] = Duty_Time;
    m24xx_read(EEPROM_24XX02,15,0,data,1);	//默认调光值
    ALLdata[4] = *data;
    ALLdata[5] = pwmtimer;
    ALLdata[6] = group_number;
    ALLdata[7] = Terminal_ID[0];
    ALLdata[8] = Terminal_ID[1];
    ALLdata[9] = Terminal_ID[2];
    ALLdata[10] = Terminal_ID[3];

    temp_buf[0]=0x7e;
    temp_buf[1]=0x0b;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF2;
    memcpy(&temp_buf[9],ALLdata,11);
    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[20]=((CRC16_value&0xff00)>>8);
    temp_buf[21]=CRC16_value&0x00ff;
    temp_buf[22]=0x0d;
    temp_buf[23]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 24 );


}


void Inquiry_lighttime(void)  //查询调光时间
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x01;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF8;
    temp_buf[9]=pwmtimer;
    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[10]=((CRC16_value&0xff00)>>8);
    temp_buf[11]=CRC16_value&0x00ff;
    temp_buf[12]=0x0d;
    temp_buf[13]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 14 );
}
void InquiryLightValue(void)//查询当前调光值
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x01;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF6;
    temp_buf[9]=Duty_Time;

    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[10]=((CRC16_value&0xff00)>>8);
    temp_buf[11]=CRC16_value&0x00ff;
    temp_buf[12]=0x0d;
    temp_buf[13]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 14 );
}

void InquiryAcquiesceLightValue(void)   //查询默认调光值
{

    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;
    temp_buf[1]=0x01;
    memcpy(&temp_buf[2],Terminal_ID,4);
    temp_buf[6]=group_number;
    temp_buf[7]=0x00;
    temp_buf[8]=0xF7;
    //     temp_buf[9]=Duty_Time;
    m24xx_read(EEPROM_24XX02,15,0,&temp_buf[9],1);
    CRC16_value=CRC16_1(temp_buf,11);
    temp_buf[10]=((CRC16_value&0xff00)>>8);
    temp_buf[11]=CRC16_value&0x00ff;
    temp_buf[12]=0x0d;
    temp_buf[13]=0x0a;
    UARTSend( (uint8_t *)temp_buf, 14 );
}


/*void ZigbeeSetup(void)           //ZIGBEE配置
{

  uint8_t temp_buf[20]={0};
   Frame_OK(0x01);

   UARTInit(38400);

   GPIOSetValue(1,8,0);

   Delay1_MS(3000);
  GPIOSetValue(1,8,1);

   temp_buf[0]=0x23;
   temp_buf[1]=0xa0;
   UARTSend(temp_buf,2);
   Delay1_MS(50);
   temp_buf[0]=0x23;
   temp_buf[1]=0xfe;
   memcpy(&temp_buf[2],&Data_Buf[8],14);

   UARTSend(temp_buf,16);
   Delay1_MS(50);
   temp_buf[0]=0x23;
   temp_buf[1]=0x23;
   UARTSend(temp_buf,2);
 //  GPIOSetValue(1,8,1);
   Delay1_MS(50);
   UARTInit(19200);
   Delay1_MS(100);

} */
void ZigbeeSetup(void)           // 读ZIGBEE配置信息
{

    uint8_t num=0;
    uint8_t temp_buf[20]= {0};
    memset(temp_buf,0,20);
    memset(Data_Buf,0,Data_Len);
    UARTInit(38400);
    NVIC_DisableIRQ(UART_IRQn);
    Delay1_MS(20);
    GPIOSetValue(1,8,0);
    Delay1_MS(3000);
    GPIOSetValue(1,8,1);
    while ((LPC_UART->LSR & 0x01) == 0x01)
    {
        num = LPC_UART->RBR;

    }
    num=0;
    Data_Buf[0]=0x23;
    Data_Buf[1]=0xa0;
    UARTSend((uint8_t *)Data_Buf,2);

    while ((LPC_UART->LSR & 0x01) == 0x01)
    {


        Data_Buf[num] = LPC_UART->RBR;
        num++;

    }
    num=0;

    memcpy(temp_buf,Data_Buf,15);
    Data_Buf[0]=0x23;
    Data_Buf[1]=0x23;
    UARTSend((uint8_t *)Data_Buf,2);
//  Delay1_MS(200);
// UARTInit(19200);
// Delay1_MS(200);


    UARTSend( (uint8_t *)temp_buf, 15 );
    Delay1_MS(20);


}
void configuration(void)        //写配置信息
{
    uint8_t buf[50];
    memset(buf,0,100);

    m24xx_write(EEPROM_24XX02, 7, 0, Version, 3);
    m24xx_write(EEPROM_24XX02, 10, 0, Terminal_ID, 4);

    buf[0]=Duty_Time;
    m24xx_write(EEPROM_24XX02,15,0,buf,1);

// buf[10]=Brate;
//  m24xx_write(EEPROM_24XX02,21,0,&buf[10],1);
// memset(buf,0,100);


    memcpy(buf,field,10);
    memcpy(&buf[11],value,10);
    buf[20]=pwmtimer;
    m24xx_write(EEPROM_24XX02, 50, 0, buf, 21);
    memset(buf,0,100);
    Frame_OK(0x06);
}
void  Baud_Rate(void)      //设置通讯波特率
{
    uint8_t b_rate;
    uint32_t rate;
    uint8_t temp[10];
    b_rate=Data_Buf[8];
    switch(b_rate)
    {
    case 0:
        rate=2400;
        break;
    case 1:
        rate=4800;
        break;
    case 2:
        rate=9600;
        break;
    case 3:
        rate=19200;
        break;
    case 4:
        rate=38400;
        break;
    case 5:
        rate=43000;
        break;
    case 6:
        rate=56000;
        break;
    case 7:
        rate=57600;
        break;
    default :
        rate=19200;
        break;
    }
    UARTInit(rate);

    temp[0]=b_rate;
    m24xx_write(EEPROM_24XX02, 21, 0, temp, 1);

}


void Delay1_MS(uint32_t ulTime)
{
    uint32_t i = 0;
    while (ulTime--)
    {
        for (i = 0; i < 5000; i++);
    }
}
void Delay1_US(uint32_t ulTime)
{
    uint32_t i = 0;
    while (ulTime--)
    {
        for (i = 0; i < 5; i++);
    }
}
void Frame_OK(uint8_t command) //回复协议
{
    uint8_t temp_buf[20];

    uint32_t CRC16_value=0;
    memset(temp_buf,0,20);
    temp_buf[0]=0x7e;                         //帧头
    temp_buf[1]=00;                           //消息长度
    memcpy(&temp_buf[2],Terminal_ID,4);        //地址
    temp_buf[6]=group_number;                 //组号
    temp_buf[7]=0x00;                          //应答码
    temp_buf[8]=(command+0XC0);                       //控制码
    CRC16_value=CRC16_1(temp_buf,8);
    temp_buf[9]=((CRC16_value&0xff00)>>8);    //校验码
    temp_buf[10]=CRC16_value&0x00ff;
    temp_buf[11]=0x0d;
    temp_buf[12]=0x0a;                        //帧尾
    UARTSend( (uint8_t *)temp_buf, 13 );
    Delay1_MS(10);
}
void heartbeat(void)    //心跳包
{
    ;
}
void upgrade(void) // 固件升级请求命令
{
    ;
}
void upgradedata(void)  //固件升级数据包
{
    ;
}
void upgradecrc(void)   //固件升级校验包
{
    ;
}
