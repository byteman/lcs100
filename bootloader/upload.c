#include "upload.h"
#include "uart.h"
#include "byprotocol.h"
#include <LPC11xx.h>
#include "crc.h"
#include "iap.h"
#include <string.h>
#define MAX_FILE_SIZE (19*1024)
#define MAX_BUFF_SIZE  256

/* Define flash memory address at which user application is located */
#define APP_START_ADDR						0x00001000UL
#define APP_END_ADDR						0x00008000UL

/* Define the flash sectors used by the application */
#define APP_START_SECTOR					1
#define APP_END_SECTOR						7

/* Define location in flash memory that contains the application valid check value */
#define APP_VALID_CHECK_ADDR				0x00007FFCUL


static uint8_t ch;
static uint8_t* pkt = 0;
static uint32_t pktLen = 0;

static uint8_t termId[4] = {0,0,0,1};
static uint8_t  cmd 	 = 0;
static uint8_t  group 	 = 0;
extern uint32_t SystemFrequency;

static uint16_t curSession = 0;
static uint8_t packet[MAX_BUFF_SIZE];
static uint8_t ctxBuf[MAX_BUFF_SIZE];


uint32_t u32BootLoader_AppPresent(void);
static uint32_t u32Bootloader_WriteCRC(uint16_t u16CRC);
static uint32_t u32BootLoader_ProgramFlash(uint8_t *pu8Data, uint16_t u16Len);
static uint32_t u32NextFlashWriteAddr = APP_START_ADDR;
static uint8_t quit = 0;

/*****************************************************************************
 ** Function name:	u32Bootloader_WriteCRC
 **
 ** Description:	Writes a 16-bit CRC value to the last location in flash
 ** 				memory, the bootloader uses this value to check for a valid
 ** 				application at startup.
 **
 ** Parameters:	    u16CRC - CRC value to be written to flash
 **
 ** Returned value: 1 if CRC written to flash successfully, otherwise 0.
 **
 *****************************************************************************/
static uint32_t u32Bootloader_WriteCRC(uint16_t u16CRC)
{
	uint32_t i;
	uint32_t u32Result = 0;
	uint32_t a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS];
	uint32_t *pu32Mem = (uint32_t *)(APP_END_ADDR - IAP_FLASH_PAGE_SIZE_BYTES);

	/* First copy the data that is currently present in the last page of
	   flash into a temporary buffer */
	for (i = 0 ; i < IAP_FLASH_PAGE_SIZE_WORDS; i++)
	{
		a32DummyData[i] = *pu32Mem++;
	}

	/* Set the CRC value to be written back */
	a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS - 1] = (uint32_t)u16CRC;

	if (u32IAP_PrepareSectors(APP_END_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
	{
		/* Now write the data back, only the CRC bits have changed */
		if (u32IAP_CopyRAMToFlash((APP_END_ADDR - IAP_FLASH_PAGE_SIZE_BYTES),
				                  (uint32_t)a32DummyData,
				                  IAP_FLASH_PAGE_SIZE_BYTES) == IAP_STA_CMD_SUCCESS)
		{
			u32Result = 1;
		}
	}
	return (u32Result);
}

/*****************************************************************************
 ** Function name:	u32BootLoader_ProgramFlash
 **
 ** Description:
 **
 ** Parameters:	    None
 **
 ** Returned value: 0 if programming failed, otherwise 1.
 **
 *****************************************************************************/
static uint32_t u32BootLoader_ProgramFlash(uint8_t *pu8Data, uint16_t u16Len)
{
	uint32_t u32Result = 0;

	if ((pu8Data != 0) && (u16Len != 0))
	{
		/* Prepare the flash application sectors for reprogramming */
		if (u32IAP_PrepareSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
		{
			/* Ensure that amount of data written to flash is at minimum the
			   size of a flash page */
			if (u16Len < IAP_FLASH_PAGE_SIZE_BYTES)
			{
				u16Len = IAP_FLASH_PAGE_SIZE_BYTES;
			}

			/* Write the data to flash */
			if (u32IAP_CopyRAMToFlash(u32NextFlashWriteAddr, (uint32_t)pu8Data, u16Len) == IAP_STA_CMD_SUCCESS)
			{
				/* Check that the write was successful */
				if (u32IAP_Compare(u32NextFlashWriteAddr, (uint32_t)pu8Data, u16Len, 0) == IAP_STA_CMD_SUCCESS)
				{
					/* Write was successful */
					u32NextFlashWriteAddr += u16Len;
					u32Result = 1;
				}
			}
		}
	}
	return (u32Result);
}

/*****************************************************************************
 ** Function name:  u32BootLoader_AppPresent
 **
 ** Description:	Checks if an application is present by comparing CRC of
 ** 				flash contents with value present at last location in flash.
 **
 ** Parameters:	    None
 **
 ** Returned value: 1 if application present, otherwise 0.
 **
 *****************************************************************************/
uint32_t u32BootLoader_AppPresent(void)
{
	uint16_t u16CRC = 0;
	uint32_t u32AppPresent = 0;
	uint16_t *pu16AppCRC = (uint16_t *)(APP_END_ADDR - 4);

	/* Check if a CRC value is present in application flash area */
	if (*pu16AppCRC != 0xFFFFUL)
	{
		/* Memory occupied by application CRC is not blank so calculate CRC of
		   image in application area of flash memory, and check against this
		   CRC.. */
		u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (APP_END_ADDR - APP_START_ADDR - 4));

		if (*pu16AppCRC == u16CRC)
		{
			u32AppPresent = 1;
		}
	}
	return u32AppPresent;
}

static void vTimerStart(uint32_t u32Periodms)
{
	/* Enable the timer clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL << 9);

	/* Configure the timer so that we can poll for a match */
	LPC_TMR32B0->TCR = 0x02;		/* reset timer */
	LPC_TMR32B0->PR  = 0x00;		/* set prescaler to zero */
	LPC_TMR32B0->MR0 = u32Periodms * ((SystemFrequency / (LPC_TMR32B0->PR + 1)) / 1000UL);
	LPC_TMR32B0->IR  = 0xFF;		/* reset all interrupts */
	LPC_TMR32B0->MCR = 0x04;		/* stop timer on match */
	LPC_TMR32B0->TCR = 0x01;		/* start timer */
}
static uint8_t vTimerTimeOut()
{
	return ((LPC_TMR32B0->TCR & 0x01) == 0)?1:0;		
}
static void vTimerStop()
{
	LPC_TMR32B0->TCR = 0x02;	
}

uint8_t sendPacket(uint8_t* context, uint32_t len)
{
    uint32_t sendlen = buildPacket(context,len,packet,512);
    //dumpData(packet,sendlen);
		vUARTSend(packet,sendlen);
    //_zigbeeCom->Write (packet,sendlen);
	return len;
}

uint8_t buildAckPacket(uint8_t code,uint8_t ack,uint16_t ssid)
{
		ctxBuf[0] = termId[0];
		ctxBuf[1] = termId[1];
		ctxBuf[2] = termId[2];
		ctxBuf[3] = termId[3];
		ctxBuf[4] = group;
		ctxBuf[5] = code;
	
		ctxBuf[6] = ack;
		ctxBuf[7] = ssid>>8;
		ctxBuf[8] = ssid&0xff;
	
		return sendPacket(ctxBuf,9);
		
}
uint8_t    parseUploadReq(uint16_t ssid,uint32_t fileSiz)
{
		
		if(fileSiz > MAX_FILE_SIZE) 
		{
				curSession = 0;
				buildAckPacket(CMD_UPLOAD_REQ,ERR_TOO_LARGE,ssid);
				return 0;
		}
			
		if (u32IAP_PrepareSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
		{
				if (u32IAP_EraseSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
				{
					//uint16_t u16CRC = 0;
					
					curSession = ssid;
					u32NextFlashWriteAddr = APP_START_ADDR; 
					buildAckPacket(CMD_UPLOAD_REQ,ERR_ERASE,ssid);
					
					return 1;
					//u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (APP_END_ADDR - APP_START_ADDR - 4));

					/* Write the CRC value into the last 16-bit location of flash, this
						 will be used to check for a valid application at startup  */
					//(void)u32Bootloader_WriteCRC(u16CRC);
				}
		}
		
		buildAckPacket(CMD_UPLOAD_REQ,ERR_OK,ssid);
		return 0;		
		
}
#define FLASH_PAGE_SIZE 256
#define PACKET_SIZE 64
__align(4)  static uint8_t au8RxBuffer[FLASH_PAGE_SIZE];

uint8_t    parseUploadData(uint16_t ssid,uint16_t idx, uint8_t* data, uint32_t len)
{
		
		if( (curSession == 0) || (ssid != curSession) || ( len != PACKET_SIZE) ) 
		{
				buildAckPacket(CMD_UPLOAD_DATA,ERR_SESSION,ssid);
				return 0;
		}
		
		idx = idx%4;
		memcpy(&au8RxBuffer[idx*FLASH_PAGE_SIZE] ,data,len);
		if(idx != 3)
		{
				buildAckPacket(CMD_UPLOAD_DATA,ERR_OK,ssid);	
				return 1;
		}


		if(u32BootLoader_ProgramFlash(au8RxBuffer,FLASH_PAGE_SIZE))
		{
				buildAckPacket(CMD_UPLOAD_DATA,ERR_OK,ssid);	
				return 1;
		}
		buildAckPacket(CMD_UPLOAD_DATA,ERR_WRITE_DATA,ssid);	
		return 0;		

}
uint8_t    parseUploadVerify(uint16_t ssid,uint16_t crc)
{
		uint16_t u16CRC = 0;
		if( (curSession == 0) || (ssid != curSession) ) 
		{
				buildAckPacket(CMD_UPLOAD_VERIFY,ERR_SESSION,ssid);
				return 0;
		}
		
		u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (APP_END_ADDR - APP_START_ADDR - 4));
		
		if(u16CRC != crc)
		{
				buildAckPacket(CMD_UPLOAD_VERIFY,ERR_CRC,ssid);
				return 0;
		}
		if(u32Bootloader_WriteCRC(u16CRC) == 0)
		{
				buildAckPacket(CMD_UPLOAD_VERIFY,ERR_WRITE_CRC,ssid);
				return 0;
		}
		buildAckPacket(CMD_UPLOAD_VERIFY,ERR_OK,ssid);
		return 1;
		
}
static uint16_t buf2uint16(uint8_t* buff)
{
		return (buff[0]<<8)+buff[1];
}

static uint16_t buf2uint32(uint8_t* buff)
{
	return (buff[0]<<24) + (buff[1]<<16) + (buff[2]<<8) + (buff[3]<<0);
}

static uint8_t parseUpload(uint8_t* buff, uint32_t len)
{
		uint8_t i = 0;
	  uint8_t ret = 0;
		for(; i < 4; i++)
		{
				if(buff[i] != termId[i]) break;
		}
		if(i != 4) return 0 ;
		
		group = buff[4];
		cmd = buff[5];
		
		switch(cmd)
		{
			case CMD_UPLOAD_REQ:
				parseUploadReq( buf2uint16(buff+6) , buf2uint32(buff+8));
				ret = 1;
 				break;
			case CMD_UPLOAD_DATA:
				parseUploadData(buf2uint16(buff+6),buf2uint16(buff+8),buff+10, len-10);
				ret = 1;
				break;
			case CMD_UPLOAD_VERIFY:
				parseUploadVerify(buf2uint16(buff+6),buf2uint16(buff+8));
				ret = 1;
				break;
			case CMD_RESET:
				quit = 1;
				break;
			default:break;
		}
		
		return ret;
}

uint8_t waitUploadRequest()
{
		vTimerStart(5000);
		quit = 0;
		while(!quit)
		{
			if(u8UARTReceive(&ch))
			{
				if(parseChar(ch))
				{
					pkt = readPacket(&pktLen);
					if(parseUpload(pkt,pktLen)) 
					{
						 //只要收到升级请求数据就复位计数器
							vTimerStart(5000);
					}
				}					
			}
			
			//if(vTimerTimeOut()) break;
			
		}
		
	  return 0;
}

void upload_task(void)
{
	protoParserInit(0);
	vUARTInit(19200);
	
	waitUploadRequest();

}

