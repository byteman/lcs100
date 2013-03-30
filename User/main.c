/***************************重庆星河光电***********************************/
#include <stdio.h>
#include "LPC11xx.h"
#include "uart.h"
#include "24C02.h"
#include "timer32.h"
#include "string.h"
#include "Application.h"
#include "CRC16_1.h"
#include "CS5463.h"
#include "gpio.h"
#include "wdt.h"
#include "tinyfifo.h"
#include "byprotocol.h"
//****************************************************************************
void Setup_Read(void);
extern uint8_t Version[3]= {0x01,0x00,0x00};
uint8_t group_number=0xFF;


uint8_t wrbuf[100]; //write//read
uint8_t Terminal_ID[4]= {0x00,0x00,0x00,0x01}; //终端ID
uint32_t Brate=19200;  //默认波特率
unsigned char* pkt = NULL;
unsigned int pktLen = 0;


/*****************************主函数*******************************************/


static u8 rxChar;
int main (void)
{
    SystemInit();
    init_time();
    UARTInit(19200);
    SSP_Init();
    GPIOInit();
    CS5463_Init();
    i2c_lpc_init(0);
    //Setup_Read();
    Delay1_MS(1000);
    UARTSend(Terminal_ID,4);
    while (1)
    {

        if(!tinyFifoEmpty())
        {
            if(tinyFifoGetc(&rxChar) == 0) //get data ok;
            {
                //UARTSend(&rxChar,1);
                //printf("0x%0x\r\n",rxChar);
                if(parseChar(rxChar))
                {
                    pkt = readPacket(&pktLen);
                    if(pkt && (pktLen < Data_Len))
                    {
                        memcpy(Data_Buf+2,pkt,pktLen);
                        if((Terminal_ID[0]==Data_Buf[2])&&(Terminal_ID[1]==Data_Buf[3])&&(Terminal_ID[2]==Data_Buf[4])&&(Terminal_ID[3]==Data_Buf[5]))
                        {
                            App_Command();
                        }
                    }
                }
            }

        }
#if 0
        if(Flag_Uart_Rx == TRUE)
        {
            Flag_Uart_Rx=FALSE;
            memcpy(Data_Buf,TempBuffer,UARTRxLength);

            UARTRxLength = 0;


            if((Terminal_ID[0]==Data_Buf[2])&&(Terminal_ID[1]==Data_Buf[3])&&(Terminal_ID[2]==Data_Buf[4])&&(Terminal_ID[3]==Data_Buf[5]))
            {
                App_Command();
            }

        }
#endif

        //              WDTFeed();

    }
}
void Setup_Read(void)
{
    uint8_t i,b_rate;
    memset(wrbuf,0,100);
    m24xx_read(EEPROM_24XX02,7,0,wrbuf,40); //从地址0x07处开始读出40个数字到rebuf
//版本信息[0,1,2]
    if(wrbuf[0]==0xff&wrbuf[1]==0xff&wrbuf[2]==0xff)
    {

    }
    else
        memcpy(Version,wrbuf,3);
//Terminal ID [3-6]
    if(wrbuf[3]==0xff&wrbuf[4]==0xff&wrbuf[5]==0xff&wrbuf[6]==0xff)
    {
    }
    else
        memcpy(Terminal_ID,&wrbuf[3],4);

//占空比[8]
    if(wrbuf[8]==0xff)
    {
        Duty_Time=100;
        init_timer32PWM(1,TIME_INTERVAL,0x01);
    }
    else
    {
        Duty_Time=wrbuf[8];
        init_timer32PWM(1,TIME_INTERVAL,0x01);
    }

//group num
    group_number=wrbuf[10];
//brate
    if(wrbuf[14]==0xff)
    {
        Brate=19200;
    }
    else
    {
        b_rate=wrbuf[14];

        switch(b_rate)
        {
        case 0:
            Brate=2400;
            break;
        case 1:
            Brate=4800;
            break;
        case 2:
            Brate=9600;
            break;
        case 3:
            Brate=19200;
            break;
        case 4:
            Brate=38400;
            break;
        case 5:
            Brate=43000;
            break;
        case 6:
            Brate=56000;
            break;
        case 7:
            Brate=57600;
            break;
        default :
            Brate=19200;
            break;
        }
    }

    memset(wrbuf,0,100);
    //场景值[50,71]
    m24xx_read(EEPROM_24XX02,50,0,wrbuf,21);
    for(i=0; i<10; i++)
    {
        if(i<10)
        {
            if(wrbuf[i]==0xff)
            {
                ;
            }
            else  field[i]=wrbuf[i];

        }
        else if(10<=i&&i<20)
        {
            if(wrbuf[i]==0xff)
            {
                ;
            }
            else   value[i]=wrbuf[i];

        }
        else if(i==20)
        {
            if(wrbuf[20]==0xff)
            {
                ;
            }
            else   pwmtimer=wrbuf[20];
        }
    }
}



/////
