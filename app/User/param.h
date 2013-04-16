#ifndef PARAM_H
#define PARAM_H
#include "LPC11xx.h"
typedef enum{
	PARAM_MAGIC=0,
	PARAM_ID=4,
	PARAM_GROUP=8,
	PARAM_BAUD=9,	//波特率
	PARAM_DEF_BRIGHTNESS=13, //亮度
	PARAM_ADJ_TIME=14,
	PARAM_MAX,
	PARAM_CRC=0xFD// 最后两个字节是前面参数的校验码
}ParamType;

extern uint8_t 	group_number;
extern uint8_t 	Terminal_ID[4];
extern uint32_t Brate;//默认波特率
extern uint32_t adj_timeS;	

extern uint8_t brightness; //当前的亮度
extern uint8_t default_brightness;//默认的亮度


uint8_t 	paramGetU8(ParamType type);
uint16_t 	paramGetU16(ParamType type);
uint32_t 	paramGetU32(ParamType type);
float 		paramGetFloat(ParamType type);
uint8_t 	paramGetBuff(ParamType type,uint8_t* buff,uint8_t size);

void 	paramSetU8(ParamType type,uint8_t value);
void 	paramSetU16(ParamType type,uint16_t value);
void 	paramSetU32(ParamType type,uint32_t value);
void 	paramSetFloat(ParamType type,float value);
void 	paramSetBuff(ParamType type,uint8_t* buff,uint8_t size);

void	loadParam(void);
void	recoveryDefaultParam(void);

#endif
