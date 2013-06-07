#include "Brightness.h"
#include "timer32.h"
#include "param.h"
#include "ledproto.h"
#include <math.h>


void RespNoPara(uint8_t command,uint8_t ack);
static int bNeedAdjust = 0;
static int bNeedShark = 0;
static int nSharkeS= 0;
static int nShark10msCounter = 0;
static int nAdj10msCounter = 0;
static int nAdjStep = 0;
static int nAdjCounter = 0;

//FIXME 到底需要闪烁多久，是一直闪到发命令停止，还是指定时间或者次数
void setTwinkle(uint16_t totalS)
{

    uint16_t cnt = 10;
    RespShortPara(CMD_TWINKLE,ERR_OK,totalS);
	
		bNeedShark = 1;
		nSharkeS = totalS;
		nShark10msCounter = 0;
		if(totalS == 0)
		{
				bNeedShark = 0;
			  PWM0_Init(brightness);
		}
   
}

/*************************调光函数**********************************************/
void adjustBrightness(uint8_t value)
{

    /* Allocate storage for CPU status register           */


    RespCharPara(CMD_ADJUST_BRIGHTNESS,ERR_OK,value);

    if( value  == brightness )
    {
        return;
    }
    if(bNeedAdjust)
    {
        //brightness = value;
        //RespNoPara(CMD_ADJUST_BRIGHTNESS,ERR_BUSY);
        //return;
    }

    //SYS_ENTER_CRITICAL();
    brightness =  value;


    nAdjStep = (value>Duty_Time)?( value-Duty_Time ):( Duty_Time - value );
    if(nAdjStep == 0)
    {
        //SYS_EXIT_CRITICAL();
        return;
    }
    nAdjCounter = (adj_timeS*100)/nAdjStep;

    if(nAdjCounter <= 0 )nAdjCounter=1;

    nAdj10msCounter = 0;
    bNeedAdjust = 1;

    //SYS_EXIT_CRITICAL();
}

void BrightnessSrv(void)
{
    if(bNeedAdjust)
    {
        nAdj10msCounter++;
        if(nAdj10msCounter < nAdjCounter) return;

        nAdj10msCounter = 0;
        if(Duty_Time > brightness) 			Duty_Time--;
        else if(Duty_Time < brightness)	Duty_Time++;

        if(Duty_Time == brightness)
        {
            bNeedAdjust = 0;
        }

        PWM0_Init(Duty_Time);



    }
		if(bNeedShark)
		{
				static int value = 100;
				if(nShark10msCounter++ < 100)
				{
						return;
				}		
        PWM0_Init(value);
				value = 100 - value;
				nShark10msCounter = 0;
				
				nSharkeS--;
				if(nSharkeS <=0)
				{
						bNeedShark = 0;
						value = 100;
						nShark10msCounter = 0;
						nSharkeS = 0;
						PWM0_Init(brightness);
				}
				
    }
}
