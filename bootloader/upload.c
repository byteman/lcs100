#include "upload.h"
#include "uart.h"
#include "byprotocol.h"
#include <LPC11xx.h>
#include "crc.h"
#include "iap.h"
#include <string.h>
#include "param.h"

/* Define the flash sectors used by the application */
#define APP_START_SECTOR					2
#define APP_END_SECTOR						7

/* Define flash memory address at which user application is located */
#define APP_START_ADDR					(APP_START_SECTOR* 0x00001000UL)
#define APP_END_ADDR						0x00008000UL



#define MAX_PAGE_SIZE 4096
#define MAX_FILE_SIZE ( (APP_END_SECTOR-APP_START_SECTOR) * MAX_PAGE_SIZE - 8) //后面8个字节存放长度和校验和
#define MAX_BUFF_SIZE  256


#define FLASH_PAGE_SIZE 256
#define PACKET_SIZE 		64

#define UPLOAD_TIMEOUT 5000
static uint8_t ch;
static uint8_t* pkt = 0;
static uint32_t pktLen = 0;


static uint8_t  cmd 	 = 0;
static uint8_t  group 	 = 0;
static uint8_t 	quit = 0;

extern uint32_t SystemFrequency;

static uint16_t curSession = 0;
static uint32_t totalSize = 0;
static uint16_t totalPktNum = 0;
static uint8_t  packet[MAX_BUFF_SIZE];
static uint8_t  ctxBuf[MAX_BUFF_SIZE];
__align(4)  static uint8_t au8RxBuffer[FLASH_PAGE_SIZE];

uint32_t u32BootLoader_AppPresent(void);
static uint32_t u32Bootloader_WriteCRC(uint32_t fileSize,uint16_t u16CRC);
static uint32_t u32BootLoader_ProgramFlash(uint8_t *pu8Data, uint16_t u16Len);
static uint32_t u32NextFlashWriteAddr = APP_START_ADDR;

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
static uint32_t u32Bootloader_WriteCRC(uint32_t fileSize,uint16_t u16CRC)
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
    a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS - 2] = (uint32_t)fileSize;

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
    uint32_t *pu32AppLen = (uint32_t *)(APP_END_ADDR - 8);
    /* Check if a CRC value is present in application flash area */
    if ( (*pu16AppCRC != 0xFFFFUL) && (*pu32AppLen < MAX_FILE_SIZE))
    {
        /* Memory occupied by application CRC is not blank so calculate CRC of
           image in application area of flash memory, and check against this
           CRC.. */
        u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (*pu32AppLen));

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
#if 0
static void vTimerStop()
{
    LPC_TMR32B0->TCR = 0x02;
}
#endif
uint8_t sendPacket(uint8_t* context, uint32_t len)
{
    uint32_t sendlen = buildPacket(context,len,packet,512);
    vUARTSend(packet,sendlen);
    return len;
}
uint8_t buildBroadTermIdAckPacket(uint8_t code,uint8_t ack)
{
    ctxBuf[0] = Terminal_ID[0];
    ctxBuf[1] = Terminal_ID[1];
    ctxBuf[2] = Terminal_ID[2];
    ctxBuf[3] = Terminal_ID[3];
    ctxBuf[4] = group;
    ctxBuf[5] = code|0x80; //slave ack

    ctxBuf[6] = ack;
    ctxBuf[7] = Terminal_ID[0];
    ctxBuf[8] = Terminal_ID[1];
    ctxBuf[9] = Terminal_ID[2];
    ctxBuf[10] = Terminal_ID[3];

    return sendPacket(ctxBuf,11);

}
uint8_t buildAckPacket(uint8_t code,uint8_t ack,uint16_t ssid)
{
    ctxBuf[0] = Terminal_ID[0];
    ctxBuf[1] = Terminal_ID[1];
    ctxBuf[2] = Terminal_ID[2];
    ctxBuf[3] = Terminal_ID[3];
    ctxBuf[4] = group;
    ctxBuf[5] = code;

    ctxBuf[6] = ack;
    ctxBuf[7] = ssid>>8;
    ctxBuf[8] = ssid&0xff;

    return sendPacket(ctxBuf,9);

}
uint8_t buildDataAckPacket(uint8_t code,uint8_t ack,uint16_t ssid,uint16_t pktidx)
{
    ctxBuf[0] = Terminal_ID[0];
    ctxBuf[1] = Terminal_ID[1];
    ctxBuf[2] = Terminal_ID[2];
    ctxBuf[3] = Terminal_ID[3];
    ctxBuf[4] = group;
    ctxBuf[5] = code;

    ctxBuf[6] = ack;
    ctxBuf[7] = ssid>>8;
    ctxBuf[8] = ssid&0xff;
    ctxBuf[9] = pktidx>>8;
    ctxBuf[10] = pktidx&0xff;
    return sendPacket(ctxBuf,11);

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

            curSession  = ssid;
            totalSize   = fileSiz;
            totalPktNum = fileSiz / PACKET_SIZE;
            u32NextFlashWriteAddr = APP_START_ADDR;
            buildAckPacket(CMD_UPLOAD_REQ,ERR_OK,ssid);
            return 1;
        }
    }

    buildAckPacket(CMD_UPLOAD_REQ,ERR_ERASE,ssid);
    return 0;

}

uint8_t    parseUploadData(uint16_t ssid,uint16_t pktIdx, uint8_t* data, uint32_t len)
{
    uint8_t offset = (pktIdx%4)&0xFF;
    uint8_t needWrite = 0;
    uint8_t ack = ERR_OK;
    if( (curSession == 0) || (ssid != curSession) || ( len != PACKET_SIZE) )
    {
        buildAckPacket(CMD_UPLOAD_DATA,ERR_SESSION,ssid);
        return 0;
    }
    if(pktIdx > totalPktNum)
    {
        buildAckPacket(CMD_UPLOAD_DATA,ERR_PACKET_NUM,ssid);
        return 0;
    }
    if( (pktIdx+1) == totalPktNum)
    {
        needWrite = 1;
    }
    memcpy(&au8RxBuffer[offset*PACKET_SIZE] ,data,len);
    if(offset != 3u)
    {
        if( (pktIdx+1) >= totalPktNum)  //最后一包数据
        {
            needWrite = 1;
            memset(&au8RxBuffer[(offset+1)*PACKET_SIZE],0,(3-offset)*PACKET_SIZE);
        }

    }
    else
    {
        needWrite = 1;
    }


    if(needWrite)
    {
        if(!u32BootLoader_ProgramFlash(au8RxBuffer,FLASH_PAGE_SIZE))
            ack = ERR_WRITE_DATA;
    }
    buildDataAckPacket(CMD_UPLOAD_DATA,ack,ssid,pktIdx);
    return 1;

}
uint8_t    parseUploadVerify(uint16_t ssid,uint16_t crc)
{
    uint16_t u16CRC = 0;
    if( (curSession == 0) || (ssid != curSession) )
    {
        buildAckPacket(CMD_UPLOAD_VERIFY,ERR_SESSION,ssid);
        return 0;
    }

    u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, totalSize);

    if(u16CRC != crc)
    {
        buildAckPacket(CMD_UPLOAD_VERIFY,ERR_CRC,ssid);
        return 0;
    }
    if(u32Bootloader_WriteCRC(totalSize,u16CRC) == 0)
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
static uint8_t broadCastDeviceID(void)
{
    buildBroadTermIdAckPacket(CMD_BROADCAST_DEVID,ERR_OK);

    return 1;
}
static uint8_t queryMode(uint8_t code,uint8_t ack)
{
	ctxBuf[0] = Terminal_ID[0];
	ctxBuf[1] = Terminal_ID[1];
	ctxBuf[2] = Terminal_ID[2];
	ctxBuf[3] = Terminal_ID[3];
	ctxBuf[4] = group;
	ctxBuf[5] = code|0x80; //slave ack
	
	ctxBuf[6] = ack;
	ctxBuf[7] = MODE_BOOT;

	return sendPacket(ctxBuf,8);

}
static uint8_t parseUpload(uint8_t* buff, uint32_t len)
{
    uint8_t i = 0;
    uint8_t ret = 0;

    if(len < 6) return 0;

    group = buff[4];
    cmd 	= buff[5];
    for(; i < 4; i++)
    {
        if( (buff[i] != Terminal_ID[i] ) && (cmd != CMD_BROADCAST_DEVID))
            break;
    }
    if(i != 4) return 0 ;



    switch(cmd)
    {
    case CMD_UPLOAD_REQ:
        ret = parseUploadReq( buf2uint16(buff+6) , buf2uint32(buff+8));
        break;
    case CMD_UPLOAD_DATA:
    {

        uint16_t sid = buf2uint16(buff+6);
        uint16_t idx = buf2uint16(buff+8);
        ret = parseUploadData(sid,idx,buff+10, len-10);
        break;
    }

    case CMD_UPLOAD_VERIFY:
        ret = parseUploadVerify(buf2uint16(buff+6),buf2uint16(buff+8));
        break;
    case CMD_RESET:
        quit = 1;
        break;
    case CMD_BROADCAST_DEVID:
        ret = broadCastDeviceID();
        break;
	case CMD_QUERY_MODE:
				ret = queryMode(CMD_QUERY_MODE,ERR_OK);
				break;
    default:
        break;
    }

    return ret;
}
void resetVar()
{
    curSession = 0;
    totalSize = 0;
    totalPktNum = 0;
    u32NextFlashWriteAddr = APP_START_ADDR;

}
void UploadService()
{
    vTimerStart(UPLOAD_TIMEOUT);
    quit = 0;

    resetVar();
    while(!quit)
    {
        if(vTimerTimeOut()) break; //超时跳出

        if(!u8UARTReceive(&ch)) continue; //没有收到数据

        if(!parseChar(ch)) continue; //不是协议包

        pkt = readPacket(&pktLen); //读出一条数据
        if(parseUpload(pkt,pktLen)) //正确的升级数据
        {
            //只要收到正确的升级数据就复位计数器
            vTimerStart(UPLOAD_TIMEOUT);
        }

    }
}

void UploadTask(void)
{
    protoParserInit(0);
    vUARTInit(19200);

    UploadService();

}

