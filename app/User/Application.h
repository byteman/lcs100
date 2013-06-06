#ifndef _Application_H
#define _Application_H


typedef struct
{
    unsigned char  id[4];
    unsigned char  group;
    unsigned char  cmd;
    unsigned char  mode;
    unsigned char  dataLen;
    unsigned char* data;

} LedRequest;

#define Data_Len 200

extern void App_Command(LedRequest* pReq);//各个命令分解
extern int getZigbeeID(void);
extern void Delay1_MS(uint32_t ulTime);
extern void Delay1_US(uint32_t ulTime);
extern unsigned short toInt(uint8_t* buf);

void ResponseMsg(uint8_t command,uint8_t ack,uint8_t* context, uint8_t len);
void RespNoPara(uint8_t command,uint8_t ack);
void RespCharPara(uint8_t command,uint8_t ack,uint8_t value);
void RespShortPara(uint8_t command,uint8_t ack,uint16_t value);
void RespIntPara(uint8_t command,uint8_t ack,uint32_t value);
#endif


