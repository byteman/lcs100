/******************** (C) COPYRIGHT 2012  ************
* 文件名: i2c.h
* 作者	: 
* 日期	: 10/18/2012
* 描述	: NXP LPC11xx 系列处理器 I2C 模块头文件.
*******************************************************************************
*******************************************************************************

*******************************************************************************/



#ifndef __I2C_H 
#define __I2C_H

//********************************************************************************
//#define Terminal_ID			0x7e000001

extern uint8_t Terminal_ID[4];
extern uint8_t group_number;
extern uint8_t field[10];
extern uint8_t value[10];
extern uint32_t pwmtimer;


//---  LPC21XX I2C flags --------------------

#define  I2C_FLAG_AA	(1<<2)
#define  I2C_FLAG_SI	(1<<3)
#define  I2C_FLAG_STO	(1<<4)
#define  I2C_FLAG_STA	(1<<5)
#define  I2C_FLAG_I2EN	(1<<6)

//---- I2C Speed
#define  I2C_SPEED_100	 0
#define  I2C_SPEED_400	 1

//--- Errors

#define  I2C_NO_ERR 				   0
#define  I2C_ERR_NO_RESPONSE		   1
#define  I2C_ERR_WRONG_PARAM		   2
#define  I2C_ERR_24XX_WR_TIMEOUT	   3
#define  I2C_ERR_24XX_BAD_ADDR		   4
#define  I2C_ERR_24XX_BAD_TYPE		   5
#define  I2C_ERR_24XX_BAD_PAGESIZE	   6
#define  I2C_ERR_24XX_WRONG_NUM 	   7



//--- EEPROM 24xx types
#define  EEPROM_24XX02                             0
#define  EEPROM_24XX04				   1
#define  EEPROM_24XX08				   2
#define  EEPROM_24XX16				   3
#define  EEPROM_24XX32				   4
#define  EEPROM_24XX64				   5
#define  EEPROM_24XX128 			   6
#define  EEPROM_24XX256 			   7
#define  EEPROM_24XX512 			   8

//--- EEPROM 24xx max addr values
#define  EEPROM_MAX_ADDR_24XX02    0x00FF	//-- 256 Bytes
#define  EEPROM_MAX_ADDR_24XX04    0x01FF	//-- 512 Bytes
#define  EEPROM_MAX_ADDR_24XX08    0x03FF	//--  1 KBytes
#define  EEPROM_MAX_ADDR_24XX16    0x07FF	//--  2 KBytes
#define  EEPROM_MAX_ADDR_24XX32    0x0FFF	//--  4 KBytes
#define  EEPROM_MAX_ADDR_24XX64    0x1FFF	//--  8 KBytes
#define  EEPROM_MAX_ADDR_24XX128   0x3FFF	//-- 16 KBytes
#define  EEPROM_MAX_ADDR_24XX256   0x7FFF	//-- 32 KBytes
#define  EEPROM_MAX_ADDR_24XX512   0xFFFF	//-- 64 KBytes

//--- EEPROM 24xx write timeout ( > 5 mS)

#define  I2C_WR_24XX_TIMEOUT	 10000

#define I2C_IDLE			0
#define I2C_STARTED 		1
#define I2C_RESTARTED		2
#define I2C_REPEATED_START	3
#define DATA_ACK			4
#define DATA_NACK			5

#define I2CONSET_I2EN		0x00000040	/* I2C Control Set Register */
#define I2CONSET_AA 		0x00000004
#define I2CONSET_SI 		0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004	/* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040

#define I2DAT_I2C			0x00000000	/* I2C Data Reg */
#define I2ADR_I2C			0x00000000	/* I2C Slave Address Reg */


#define I2CONSET_I2EN		0x00000040	/* I2C Control Set Register */
#define I2CONSET_AA 		0x00000004
#define I2CONSET_SI 		0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004	/* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040

#define I2DAT_I2C			0x00000000	/* I2C Data Reg */
#define I2ADR_I2C			0x00000000	/* I2C Slave Address Reg */
#define I2SCLH_SCLH 		0x00000180	/* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL 		0x00000180	/* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000020	/* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000020	/* Fast Plus I2C SCL Duty Cycle Low Reg */


extern uint8_t wrbuf[100]; //write//read



//---- Prototypes ---

void i2c_lpc_init(int Mode);

int m24xx_write(
	int eeprom_type,	//-- EEPROM type
	int eeprom_addr,	//-- start eeprom addr ( not included Hardware A2,A1,A0)
	int eeprom_cs_val,	//-- Hardware A2,A1,A0 (valid from 24XX32)
	unsigned char *buf, 		//-- Data src buf
	int num);			//-- Bytes to write qty

int m24xx_read(
  int eeprom_type,	  //-- EEPROM type
  int eeprom_addr,	  //-- start eeprom addr ( not included Hardware A2,A1,A0)
  int eeprom_cs_val,  //-- Hardware A2,A1,A0 (valid from 24XX32)
  unsigned char *buf,		  //-- Data dst buf
  int num); 		  //-- Bytes to read qty

void i2c_lpc_init(int Mode);
extern void Setup_Write(void);
extern void Setup_Read(void);

//********************************************************************************

#endif /* end __I2C_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/************* (C) COPYRIGHT 2010 Wuhan R&D Center, Embest *****文件结束*******/

/*********************************************************************************************************
** End Of File
*********************************************************************************************************/

