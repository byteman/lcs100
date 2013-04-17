#ifndef _Application_H
#define _Application_H


typedef struct{
	unsigned char  id[4];
	unsigned char  group;
	unsigned char  cmd;
    unsigned char  mode;
	unsigned char  dataLen;
	unsigned char* data;
	
}LedRequest;

#define Data_Len 200

extern void App_Command(LedRequest* pReq);//各个命令分解

extern void Delay1_MS(uint32_t ulTime);
extern void Delay1_US(uint32_t ulTime);

#endif


