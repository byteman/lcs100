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
//****************************************************************************
void Setup_Read(void);

unsigned char* pkt = NULL;
unsigned int pktLen = 0;


/*****************************主函数*******************************************/


static u8 rxChar;
static LedRequest request;
int main (void)
{
    SystemInit();
    init_time();
    UARTInit(19200);
    SSP_Init();
    GPIOInit();
    CS5463_Init();
    i2c_lpc_init(0);
    loadParam();

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

                        App_Command(&request);

                    }
                }
            }

        }
    }
}
void Setup_Read(void)
{


}



/////
