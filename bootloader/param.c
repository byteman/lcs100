#include "24C02.h"
#include "param.h"

#define MAGIC_VALID 0x55AAFE7D
#define GROUP_DEFALUT 0x0
#define ID1_DEFAULT 0x00
#define ID2_DEFAULT 0x00
#define ID3_DEFAULT 0x00
#define ID4_DEFAULT 0x01

#define ID_DEFAULT {ID1_DEFAULT,ID2_DEFAULT,ID3_DEFAULT,ID4_DEFAULT}

#define BAUD_DEFAULT 19200

#define ADJ_TIME_DEFAULT 20
#define BRIGHT_DEFAULT 100


uint8_t group_number=GROUP_DEFALUT;
uint8_t Terminal_ID[4]= ID_DEFAULT; //终端ID
uint32_t Brate=BAUD_DEFAULT;  //默认波特率
uint32_t adj_timeS=ADJ_TIME_DEFAULT;	
uint8_t brightness=BRIGHT_DEFAULT; //当前的亮度
uint8_t default_brightness=BRIGHT_DEFAULT;//默认的亮度



static uint8_t checkBuf[PARAM_MAX];

uint8_t paramGetU8(ParamType type)
{
	uint8_t value = 0;
	m24xx_read(EEPROM_24XX02,type,0,&value,sizeof(uint8_t)); 
	return value;
}
uint16_t paramGetU16(ParamType type)
{
	uint16_t value = 0;
	m24xx_read(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(uint16_t)); 
	return value;
}
uint32_t paramGetU32(ParamType type)
{
	uint32_t value = 0;
	m24xx_read(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(uint32_t)); 
	return value;
}
float paramGetFloat(ParamType type)
{
	float value = 0;
	m24xx_read(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(float));
	return value;
}
uint8_t paramGetBuff(ParamType type,uint8_t* buff,uint8_t size)
{
	
	m24xx_read(EEPROM_24XX02,type,0,buff,size); 
	
	return size;
}


void paramSetU8(ParamType type,uint8_t value)
{
    m24xx_write(EEPROM_24XX02,type,0,&value,sizeof(uint8_t));
}

void paramSetU16(ParamType type,uint16_t value)
{
    m24xx_write(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(uint16_t));

}

void paramSetU32(ParamType type,uint32_t value)
{
    m24xx_write(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(uint32_t));

}
void paramSetFloat(ParamType type,float value)
{
    m24xx_write(EEPROM_24XX02,type,0,(unsigned char*)&value,sizeof(float));

}
void paramSetBuff(ParamType type,uint8_t* buff,uint8_t size)
{
    m24xx_write(EEPROM_24XX02,type,0,buff,size);
}
//对所有参数进行CRC16校验
static uint8_t checkValidData()
{
	return 1;
}
void	loadParam()
{
	uint32_t magic = paramGetU32(PARAM_MAGIC);
	if(magic != MAGIC_VALID)
	{
		recoveryDefaultParam();
		return;
	}

	paramGetBuff(PARAM_ID,Terminal_ID,4);
	group_number = paramGetU8(PARAM_GROUP);
	//Brate = paramGetU32(PARAM_BAUD);
	
	adj_timeS=paramGetU8(PARAM_ADJ_TIME);

	default_brightness = paramGetU8(PARAM_DEF_BRIGHTNESS);
	brightness = default_brightness;

}
void	recoveryDefaultParam()
{

	Terminal_ID[0] = ID1_DEFAULT;
	Terminal_ID[1] = ID2_DEFAULT;
	Terminal_ID[2] = ID3_DEFAULT;
	Terminal_ID[3] = ID4_DEFAULT;
	
	paramSetBuff(PARAM_ID,Terminal_ID,4);
	
	group_number = GROUP_DEFALUT;
	paramSetU8(PARAM_GROUP,group_number);
	
	//Brate = paramGetU32(PARAM_BAUD);
	
	adj_timeS= ADJ_TIME_DEFAULT;
	paramSetU8(PARAM_ADJ_TIME,adj_timeS);

	default_brightness= BRIGHT_DEFAULT;
	paramSetU8(PARAM_DEF_BRIGHTNESS,default_brightness);

	brightness = default_brightness;
	
	Brate = BAUD_DEFAULT;
	paramSetU32(PARAM_BAUD,Brate);
	
	paramSetU32(PARAM_MAGIC,MAGIC_VALID);
}
