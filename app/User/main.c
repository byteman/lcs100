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
#include "param.h"
#include "Brightness.h"
#include "timer16.h"
//****************************************************************************
void Setup_Read(void);

unsigned char* pkt = NULL;
unsigned int pktLen = 0;
extern volatile uint32_t bFlag10ms;
extern volatile uint32_t bFlag1s;
void led_flash(void);
/*****************************主函数*******************************************/


static u8 rxChar;
static LedRequest request;
int main (void)
{
    SystemInit();
    UARTInit(19200);
    SSP_Init();
    GPIOInit();
    CS5463_Init();
    i2c_lpc_init(0);
    loadParam();

    //load参数后才开始设置亮度

    PWM0_Init(brightness);

    init_timer16(0,8);
    enable_timer16(0);

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
                        //memcpy(Data_Buf,pkt,pktLen);
                        memcpy(request.id, pkt, 4);
                        request.group = pkt[4];
                        request.cmd = 	pkt[5]&0x3F;
                        request.mode = 	pkt[5]>>6;
                        request.data = 	pkt+6;

                        request.dataLen = pktLen - 3;
											
												if(request.mode == 0) //只处理主设备发来的数据包
                        {
														App_Command(&request);
												}

                    }
                }
            }

        }
        if(bFlag10ms) //10ms
        {
            BrightnessSrv();
            bFlag10ms = 0;
						
        }
				
				if(bFlag1s)
				{
					#if 0
					  led_flash();
					#endif
				}

    }
}
void Setup_Read(void)
{


}



/////
