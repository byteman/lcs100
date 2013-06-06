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
#include "LedProto.h"
#include "param.h"
#include "cs5463.h"
#include "Brightness.h"

#define MAX_RESP_BUFF_SIZE  128


enum
{
    MODE_UNICAST=0,
    MODE_GROUP=1
};
uint8_t Data_Buf[Data_Len];

const int LedVersion __attribute__((at(0x03000)))=109; 	  //1.00版本 0.01 - 2.53
uint8_t Command;
uint8_t Mode;

static uint8_t respBuf[MAX_RESP_BUFF_SIZE];
int getZigbeeID(void);

#if 1
void led_init(void)
{
		//LPC_IOCON->PIO0_1 &= ~0x07; 										/* 将P1.9初始化为GPIO功能,继电器		*/
		//LPC_IOCON->PIO0_1 |= 0x01;
    GPIOSetDir(0,1,1);
}
void led_on()
{
	GPIOSetValue(0,1,1);
}
void led_off()
{
	GPIOSetValue(0,1,0);
}
void led_flash(void)
{
	 static uint8_t state = 0;
	 GPIOSetValue(0,1,state);
	 state = !state;
}
#endif

unsigned short toShort(uint8_t* buf)
{
    return (buf[0]<<8) + buf[1];
}
unsigned short toInt(uint8_t* buf)
{
    return (buf[0]<<24) + (buf[1]<<16) +(buf[2]<<8) +(buf[3]<<0);
}
extern volatile  uint32_t timer32_0_counter;

//回复协议


/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
unsigned int
_htonl(unsigned int n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
unsigned int
_ntohl(unsigned int n)
{
    return _htonl(n);
}

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
unsigned short
_htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
unsigned short
_ntohs(unsigned short n)
{
    return _htons(n);
}


static void resetCtrl(uint16_t afterMs)
{

    RespNoPara(CMD_RESET,ERR_OK);
    Delay1_MS(afterMs);
    NVIC_SystemReset();
}


void SetupAdjustTime(uint8_t waitS)//设置调光时间
{
    adj_timeS=waitS;

    RespCharPara(CMD_SET_ADJ_TIME,ERR_OK,waitS);

    paramSetU8(PARAM_ADJ_TIME,waitS);



}

void SetupDefaultBrightness(uint8_t brightness)//设置默认调光值
{
    default_brightness=brightness;

    RespCharPara(CMD_SET_DEFAULT_BRIGHTNESS,ERR_OK,brightness);

    paramSetU8(PARAM_DEF_BRIGHTNESS,brightness);

}
void SetupGroupNumber(uint8_t group)//设置组号
{
    group_number=group;


    RespCharPara(CMD_SET_GROUP,ERR_OK,group);

    paramSetU8(PARAM_GROUP,group_number);
}
static void queryVoltage(void)
{

    uint32_t voltage =Measuring_220V();

    RespIntPara(CMD_QUERY_VOLTAGE,ERR_OK,voltage);

}
static void queryCurrent(void)
{

    uint32_t currentMA =Measuring_AC(); //以MA为单位

    RespIntPara(CMD_QUERY_CURRENT,ERR_OK,currentMA);

}
static void queryPactive(void)
{

    uint32_t value =Measuring_Pactive(); //单位为100MW

    RespIntPara(CMD_QUERY_KW,ERR_OK,value);

}
void Modify_ID(unsigned char* pId)    //修改设备ID号
{
    uint32_t value = toInt(pId);

    RespIntPara(CMD_MODIFY_DEVID,ERR_OK,value);

    memcpy(Terminal_ID,pId,4);
    paramSetBuff(PARAM_ID,Terminal_ID,4);

}
void InquiryGroupNumber(void)//查询组号
{
    RespCharPara(CMD_QUERY_GROUP,ERR_OK,group_number);
}
void InquiryLightValue(void)//查询当前调光值
{
    RespCharPara(CMD_QUERY_BRIGHTNESS,ERR_OK,Duty_Time);
}
void InquiryAcquiesceLightValue(void)   //查询默认调光值
{
    RespCharPara(CMD_QUERY_DEFAULT_BRIGHTNESS,ERR_OK,default_brightness);
}
void Inquiry_Version(void)//查询固件版本号
{
    RespCharPara(CMD_QUERY_VERSION,ERR_OK,LedVersion);
}
void Inquiry_ResetNum(void)
{
    RespIntPara(CMD_GET_RESET_CNT,ERR_OK,resetNum);
}

void Inquiry_lighttime(void)  //查询调光时间
{
    RespCharPara(CMD_QUERY_ADJ_TIME,ERR_OK,adj_timeS);
}
void BroadCastDeviceID(void)
{
    ResponseMsg(CMD_BROADCAST_DEVID,ERR_OK,(uint8_t*)Terminal_ID,4);
}
void Inquiry_ZigbeeCfg()
{

}

void Inquiry_alldata(void)   //查询所有数据
{

    uint8_t ALLdata[32];
    uint32_t temp = Measuring_220V();

    ALLdata[0] = temp>>24;
    ALLdata[1] = temp>>16;
    ALLdata[2] = temp>>8;
    ALLdata[3] = temp>>0;


    temp = Measuring_AC();

    ALLdata[4] = temp>>24;
    ALLdata[5] = temp>>16;
    ALLdata[6] = temp>>8;
    ALLdata[7] = temp>>0;


    temp = Measuring_Pactive();

    ALLdata[8]  = temp>>24;
    ALLdata[9]  = temp>>16;
    ALLdata[10] = temp>>8;
    ALLdata[11] = temp>>0;


    ALLdata[12] = brightness;
    ALLdata[13] = default_brightness;
    ALLdata[14] = adj_timeS;
    ALLdata[15] = group_number;
    ALLdata[16] = Terminal_ID[0];
    ALLdata[17] = Terminal_ID[1];
    ALLdata[18] = Terminal_ID[2];
    ALLdata[19] = Terminal_ID[3];
    ALLdata[20] = LedVersion&0xFF;

    ALLdata[21] = (resetNum>>24)&0xFF;
    ALLdata[22] = (resetNum>>16)&0xFF;
    ALLdata[23] = (resetNum>>8)&0xFF;
    ALLdata[24] = resetNum&0xFF;


    ResponseMsg(CMD_QUERY_ALL,ERR_OK,ALLdata,25);

}

void Write2EEPROM(unsigned char* buff, int len)
{

}

void Inquiry_Mode(void)
{
    RespCharPara(CMD_QUERY_MODE,ERR_OK,MODE_APP);
}

void clearResetCounter(void)
{
		resetNum = 0;
    paramSetU32(PARAM_RESET_NUM,resetNum);
	  RespIntPara(CMD_CLEAR_RESET_NUM,ERR_OK,resetNum);
}

__asm void runApp()
{

	ldr r0, =0x0
	ldr r0, [r0]
	mov sp, r0

	/* Load program counter with application reset vector address, located at
	second word of application area. */
	ldr r0, =0x4
	ldr r0, [r0]
	bx  r0

}
__asm  void enable_irq()
{

	cpsie i;


}
__asm  void disable_irq()
{
	cpsid i;

}
void App_Command(LedRequest* pReq)//各个命令分解
{
    unsigned char* data = pReq->data;
    Command=pReq->cmd;
    Mode = MODE_UNICAST;


    if(pReq->cmd == CMD_BROADCAST_DEVID)
    {
        goto cmd;
    }


    if(pReq->group == 0)
    {
        if(Terminal_ID[0] != pReq->id[0]) return;
        if(Terminal_ID[1] != pReq->id[1]) return;
        if(Terminal_ID[2] != pReq->id[2]) return;
        if(Terminal_ID[3] != pReq->id[3]) return;
    }
    else
    {
        if(pReq->group != group_number)
            return; //组号优先
        Mode = MODE_GROUP;
    }
cmd:

    switch(Command)
    {
    case CMD_UPLOAD_REQ:
        //resetCtrl(10);
				NVIC_DisableIRQ(UART_IRQn);
				NVIC_DisableIRQ(TIMER_16_0_IRQn);
				runApp();

				NVIC_EnableIRQ(UART_IRQn);
				NVIC_EnableIRQ(TIMER_16_0_IRQn);
        break;
    case CMD_RESET: //复位
        resetCtrl(toShort(data));
        break;
    case CMD_ADJUST_BRIGHTNESS: //调节亮度
        adjustBrightness(data[0]);
        break;
    case CMD_TWINKLE: //闪烁
        setTwinkle(toShort(data));
        break;
    case CMD_SET_ADJ_TIME: //设置调光时间
        SetupAdjustTime(data[0]);
        break;
    case CMD_SET_DEFAULT_BRIGHTNESS:
        SetupDefaultBrightness(data[0]);
        break;
    case CMD_SET_GROUP:
        SetupGroupNumber(data[0]);
        break;
    case CMD_MODIFY_DEVID:
        Modify_ID(data);
        break;
    case CMD_QUERY_VOLTAGE:
        queryVoltage();
        break;
    case CMD_QUERY_CURRENT:
        queryCurrent();
        break;
    case CMD_QUERY_KW:
        queryPactive();
        break;
    case CMD_QUERY_BRIGHTNESS:
        InquiryLightValue();
        break;
    case CMD_QUERY_DEFAULT_BRIGHTNESS:
        InquiryAcquiesceLightValue();
        break;
    case CMD_QUERY_GROUP:
        InquiryGroupNumber();
        break;
    case CMD_QUERY_ALL:
        Inquiry_alldata();
        break;
    case CMD_QUERY_ADJ_TIME:
        Inquiry_lighttime();
        break;
    case CMD_QUERY_ZIGBEE_CFG:
        Inquiry_ZigbeeCfg();
        break;
    case CMD_WRITE_EEPROM:
        Write2EEPROM(data,10);
        break;
    case CMD_BROADCAST_DEVID:
        BroadCastDeviceID();
        break;
    case CMD_QUERY_VERSION:
        Inquiry_Version();
        break;
    case CMD_GET_RESET_CNT:
        Inquiry_ResetNum();
        break;
    case CMD_QUERY_MODE:
        Inquiry_Mode();
        break;
		case CMD_CLEAR_RESET_NUM:
				clearResetCounter();
    default:
        break;
    }

}

void ZigbeeSetup(void)          // 读ZIGBEE配置信息
{

    uint8_t num=0;
    uint8_t temp_buf[20]= {0};
    memset(temp_buf,0,20);
    //memset(Data_Buf,0,Data_Len);
    UARTInit(38400);
    NVIC_DisableIRQ(UART_IRQn);
    Delay1_MS(20);
    GPIOSetValue(1,8,0);
    Delay1_MS(3000);
    GPIOSetValue(1,8,1);
    Delay1_MS(500);
    while ((LPC_UART->LSR & 0x01) == 0x01)
    {
        num = LPC_UART->RBR;

    }
    num=0;
#if 1
    temp_buf[0]=0x23;
    temp_buf[1]=0xa0;
    UARTSend((uint8_t *)temp_buf,2);
    Delay1_MS(500);
    while ((LPC_UART->LSR & 0x01) == 0x01)
    {

        temp_buf[num] = LPC_UART->RBR;
        num++;

    }
    //检测返回的数据是否是15个字节
    if(num != 15) return;
#endif
//后面的这两个命令zigbee都不会返回数据
    temp_buf[0] = 0x23;
    temp_buf[1] = 0xfe;
    memcpy(temp_buf+2,Data_Buf+8,14);

    UARTSend((uint8_t *)temp_buf,16);
    Delay1_MS(500);

    temp_buf[0]=0x23;
    temp_buf[1]=0x23;
    UARTSend((uint8_t *)temp_buf,2);
    Delay1_MS(200);

    UARTInit(19200); //open interrput in init

// Delay1_MS(200);



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
uint8_t checkSum(uint8_t* buff, uint8_t len)
{
    uint8_t sum = 0;
    uint8_t i = 0;
    for(; i < len; i++)
        sum+=buff[i];
    return sum;
}

void RespCharPara(uint8_t command,uint8_t ack,uint8_t value)
{
    ResponseMsg(command,ack,&value,1);
}
void RespShortPara(uint8_t command,uint8_t ack,uint16_t value)
{
    value = _htons(value);
    ResponseMsg(command,ack,(unsigned char*)&value,2);
}
void RespIntPara(uint8_t command,uint8_t ack,uint32_t value)
{
    value = _htonl(value);
    ResponseMsg(command,ack,(unsigned char*)&value,4);
}
void RespNoPara(uint8_t command,uint8_t ack)
{
    ResponseMsg(command,ack,0,0);
}
void ResponseMsg(uint8_t command,uint8_t ack,uint8_t* context, uint8_t len) //回复协议
{
    if(Mode == MODE_GROUP) return;

    if( (10 + len) > MAX_RESP_BUFF_SIZE ) return;


    respBuf[0]=PROTO_HEAD;                         //帧头
    respBuf[1]=10 + len;                           //消息长度


    memcpy(&respBuf[2],Terminal_ID,4);        //地址
    respBuf[6]=group_number;                 //组号

    respBuf[7]=(command&0x3F)|0x80;                       //控制码
    respBuf[8]= ack;                          //应答码

    if( (context != NULL) && (len > 0 ))
        memcpy(respBuf+9,context,len);

    respBuf[9+len]=checkSum(respBuf,9+len);

    UARTSend( (uint8_t *)respBuf, respBuf[1] );
    //Delay1_MS(10);

}

int getZigbeeID(void)
{
#define MAX_TMP_SIZE 128
    int zigbeeID = -1;
    uint8_t num=0;
    uint8_t tmpChar = 0;
    uint8_t temp_buf[MAX_TMP_SIZE]= {0};

    memset(temp_buf,0,MAX_TMP_SIZE);

    UARTInit(38400);
    NVIC_DisableIRQ(UART_IRQn);
    Delay1_MS(1);
    GPIOSetValue(1,8,0);
    Delay1_MS(2000);
    GPIOSetValue(1,8,1);

    Delay1_MS(100);//等待zigbee模块返回数据
    while ((LPC_UART->LSR & 0x01) == 0x01)
    {
        tmpChar = LPC_UART->RBR;
        num++;
    }
    num=0;

    temp_buf[0]=0x23;
    temp_buf[1]=0xa0;
    UARTSend((uint8_t *)temp_buf,2);
    Delay1_MS(300); //等待zigbee模块返回数据
    while ( ( (LPC_UART->LSR & 0x01) == 0x01 ) && (num < MAX_TMP_SIZE))
    {
        temp_buf[num] = LPC_UART->RBR;
        num++;
    }
    //检测返回的数据是否是15个字节
    if( ( num != 15 ) || ( temp_buf[0] == 0xA2 ) )
    {
				UARTInit(19200); //open interrput in init
        NVIC_EnableIRQ(UART_IRQn);
        return -1;
    }

    zigbeeID = (temp_buf[1]<<8) + temp_buf[2];
    Delay1_MS(5);

//复位设备
    temp_buf[0]=0x23;
    temp_buf[1]=0x23;
    UARTSend((uint8_t *)temp_buf,2);

    Delay1_MS(100); //等待数据发送完毕

    UARTInit(19200); //open interrput in init

    NVIC_EnableIRQ(UART_IRQn);

    return zigbeeID;


#if 0
//后面的这两个命令zigbee都不会返回数据
    temp_buf[0] = 0x23;
    temp_buf[1] = 0xfe;
    memcpy(temp_buf+2,Data_Buf+8,14);

    UARTSend((uint8_t *)temp_buf,16);
    Delay1_MS(500);

    temp_buf[0]=0x23;
    temp_buf[1]=0x23;
    UARTSend((uint8_t *)temp_buf,2);
    Delay1_MS(200);

    UARTInit(19200); //open interrput in init
#endif
}
