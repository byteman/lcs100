/******************** (C) COPYRIGHT 2010 Embest Info&Tech Co.,LTD. ************
* 文件名: i2c.c
* 作者	: Wuhan R&D Center, Embest
* 日期	: 01/10/2013
* 描述	: NXP LPC11xx 系列处理器 I2C API 文件.
*******************************************************************************
*******************************************************************************
* 历史:
* 01/10/2013		 : V1.0.0 	   初始版本
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "LPC11xx.h"			/* LPC11xx 外设寄存器 */
#include "24C02.h"
#include <stdio.h>
#include "gpio.h"
/**************************************i2c初始化******************************/

void i2c_lpc_init(int Mode)
{
    LPC_SYSCON->PRESETCTRL |= (0x1<<1);


    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5); //ENABLE I2C
    LPC_IOCON->PIO0_4 &= ~0x3F;	/*	I2C I/O config */
    LPC_IOCON->PIO0_4 |= 0x01;		/* I2C SCL */
    LPC_IOCON->PIO0_5 &= ~0x3F;
    LPC_IOCON->PIO0_5 |= 0x01;		/* I2C SDA */

    /*--- Clear flags ---*/
    LPC_I2C->CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;

    LPC_I2C->SCLL   = I2SCLL_SCLL;
    LPC_I2C->SCLH   = I2SCLH_SCLH;
    if(Mode == I2C_SPEED_400)
    {
        //--- I2C Timing for 48 MHz (t = 16.954 ns) ---
        LPC_I2C->SCLH = 47;  //-- more then 0.6 us  - 0.8
        LPC_I2C->SCLL = 93;  //-- more then 1.3 us  - 1.4
    }
    else //Slow
    {
        LPC_I2C->SCLH = 47*5;
        LPC_I2C->SCLL = 93*5;
    }

    LPC_I2C->CONCLR = 0xFF;			 //-- Clear all flags
    LPC_I2C->CONSET = 0x40;			 //-- Set Master Mode
    LPC_I2C->CONSET |= I2CONSET_I2EN; //--- Enable I2C

}

/********************************i2c的读写************************************/

//---------------------------------------------------------------------------
static void i2c_lpc_wr_byte(int byte)
{
    LPC_I2C->DAT = byte;
    LPC_I2C->CONCLR = I2C_FLAG_SI;				 //-- Clear SI
    while(!(LPC_I2C->CONSET & I2C_FLAG_SI)); 	 //-- End wr POINT
}

//---------------------------------------------------------------------------
static void i2c_lpc_stop()
{
    //-- Set STOP condition
    LPC_I2C->CONCLR = I2C_FLAG_SI;				   //-- Clear SI
    LPC_I2C->CONSET |=  I2C_FLAG_AA | I2C_FLAG_STO; //-- Clear NO ASK
}

//---------------------------------------------------------------------------
static int i2c_lpc_ctrl(int ctrl)
{
    int chk;
    //-- Set START
    LPC_I2C->CONCLR = 0xFF; // Clear all bits
    LPC_I2C->CONSET |= I2C_FLAG_I2EN | I2C_FLAG_STA;
    while(!(LPC_I2C->CONSET & I2C_FLAG_SI)); 	 //--- End START
    //-- Set ADDRESS
    LPC_I2C->DAT = ctrl;
    LPC_I2C->CONCLR = I2C_FLAG_STA | I2C_FLAG_SI; //-- Clear START & SI
    if(ctrl & 1) //-- RD
        chk = 0x40; //-- 40H - SLA+R has been transmitted; ACK has been received
    else
        chk = 0x18; //-- 18H - SLA+W has been transmitted; ACK has been received
    while(!(LPC_I2C->CONSET & I2C_FLAG_SI)); 	 //-- End CTRL
    if(LPC_I2C->STAT != chk)
    {
        i2c_lpc_stop();
        return I2C_ERR_NO_RESPONSE;
    }
    return I2C_NO_ERR;
}

//---------------------------------------------------------------------------
static int i2c_lpc_rx_to_buf(unsigned char *buf,int num)
{
    int rc;

    if(buf == NULL)
        return I2C_ERR_WRONG_PARAM;

    rc = num;
    if(rc > 1)
    {
        LPC_I2C->CONCLR = I2C_FLAG_SI;
        LPC_I2C->CONSET |= I2C_FLAG_AA;
        for(;;)
        {
            while(!(LPC_I2C->CONSET & I2C_FLAG_SI));  //-- End Data from slave;
            *buf++ = (unsigned char)LPC_I2C->DAT;
            rc--;
            if(rc <= 0)
                break;
            else if(rc == 1)
                LPC_I2C->CONCLR = I2C_FLAG_AA | I2C_FLAG_SI;  //-- After next will NO ASK
            else
            {
                LPC_I2C->CONCLR = I2C_FLAG_SI;
                LPC_I2C->CONSET |= I2C_FLAG_AA;
            }
        }
    }
    else if(rc == 1)
    {
        LPC_I2C->CONCLR = I2C_FLAG_AA | I2C_FLAG_SI;	//-- After next will NO ASK
        while(!(LPC_I2C->CONSET & I2C_FLAG_SI));	//-- End Data from slave;
        *buf = (unsigned char)LPC_I2C->DAT;
    }
    else //err
        return I2C_ERR_WRONG_PARAM;

    return I2C_NO_ERR;
}
//----------------------------------------------------------------------
static int i2c_lpc_ask_polling_op(int ctrl)  //-- wait until write finished
{
    int rc;
    int i;


    for(i=0; i < I2C_WR_24XX_TIMEOUT; i++) //-- actually wr = ~110 but timeout =10000

    {
        LPC_I2C->CONSET = I2C_FLAG_STA;
        LPC_I2C->CONCLR = I2C_FLAG_SI;  //-- Here - clear only SI (not all LPC_I2C->I2CONCLR)
        while(!(LPC_I2C->CONSET & I2C_FLAG_SI));		  //wait the ACK

        LPC_I2C->DAT = ctrl & 0xFE;; // R/WI = 0
        LPC_I2C->CONCLR = I2C_FLAG_SI | I2C_FLAG_STA; //-- Clear START & SI
        while(!(LPC_I2C->CONSET & I2C_FLAG_SI));//wait the ACK
        rc = LPC_I2C->STAT;
        if(rc == 0x18) //-- got ACK after CLA + W
            break;
        else
        {

        }
    }
    if(i == I2C_WR_24XX_TIMEOUT)
        return I2C_ERR_24XX_WR_TIMEOUT;
    return I2C_NO_ERR;
}
//===========================================================================
//================ EEPROM 24XX series ========================================
//===========================================================================

//----------------------------------------------------------------------------
static int m24xx_page_size(int eeprom_type, int eeprom_addr,int * pg_sz)
{
    int page_size,rc;

    page_size = 0;
    rc = I2C_NO_ERR;
    switch(eeprom_type)
    {
    case EEPROM_24XX02:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX02)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 8; // 24LC02B have page size = 8,24C04A = 8
        break;
    case EEPROM_24XX04:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX04)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 16; // 24LC04B have page size = 16,24C04A = 8
        break;
    case EEPROM_24XX08:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX08)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 16;
        break;
    case EEPROM_24XX16:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX16)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 16;
        break;
    case EEPROM_24XX32:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX32)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 32;
        break;
    case EEPROM_24XX64:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX64)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 32;
        break;
    case EEPROM_24XX128:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX128)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 64;
        break;
    case EEPROM_24XX256:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX256)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 64;
        break;
    case EEPROM_24XX512:
        if(eeprom_addr > EEPROM_MAX_ADDR_24XX512)
            rc = I2C_ERR_24XX_BAD_ADDR;
        else
            page_size = 128;
        break;
    }
    if(rc != I2C_NO_ERR)
        return rc;
    if(page_size == 0)  //-- Bad eeprom_type
        return I2C_ERR_24XX_BAD_TYPE;

    if(pg_sz)
        *pg_sz = page_size;
    return I2C_NO_ERR;
}
//----------------------------------------------------------------------------
static int m24xx_set_addr(
    int eeprom_type,	 //-- EEPROM type
    int eeprom_addr,	 //-- start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,	 //-- Hardware A2,A1,A0 (valid from 24XX32)
    int * ctrl_val, 	 //-- Value of ctrl(return)
    int * addr_hi_val,	 //-- Value of addr_hi(return)
    int * addr_lo_val)	 //-- Value of addr_lo(return)
{
    int ctrl;
    int addr_hi;
    int addr_lo;
    int rc;

    rc = I2C_NO_ERR;
    ctrl = 0;
    addr_hi = 0;
    addr_lo = 0;

    switch(eeprom_type)
    {
    case EEPROM_24XX02:	// 24LC02B ignore AO,A1,A2 pins


        ctrl = (eeprom_addr>>7) & 0x00; //-- 00000010
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = eeprom_addr & 0xFF;
        addr_lo = -1;
        break;

    case EEPROM_24XX04:	// 24LC04B ignore AO,A1,A2 pins


        ctrl = (eeprom_addr>>7) & 0x02; //-- 00000010
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = eeprom_addr & 0xFF;
        addr_lo = -1;
        break;

    case EEPROM_24XX08: // 24LC08B ignore AO,A1,A2 pins

        ctrl = (eeprom_addr>>7) & 0x06; //-- 00000110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = eeprom_addr & 0xFF;
        addr_lo = -1;
        break;

    case EEPROM_24XX16: // 24LC16B ignore AO,A1,A2 pins

        //ctrl = (eeprom_addr>>8) & 0x07; //-- 00001110
        ctrl = (eeprom_addr>>7) & 0x0e;  //BUG已经消除
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = eeprom_addr & 0xFF;
        addr_lo = -1;
        break;

    case EEPROM_24XX32:

        ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (eeprom_addr>>8) & 0x0F;
        addr_lo = eeprom_addr & 0xFF;
        break;

    case EEPROM_24XX64:

        ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (eeprom_addr>>8) & 0x1F;
        addr_lo = eeprom_addr & 0xFF;
        break;

    case EEPROM_24XX128:

        ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (eeprom_addr>>8) & 0x3F;
        addr_lo = eeprom_addr & 0xFF;
        break;

    case EEPROM_24XX256:

        ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (eeprom_addr>>8) & 0x7F;
        addr_lo = eeprom_addr & 0xFF;
        break;

    case EEPROM_24XX512:

        ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (eeprom_addr>>8) & 0xFF;
        addr_lo = eeprom_addr & 0xFF;
        break;
    }
    if(rc != I2C_NO_ERR)
        return rc;
    if(ctrl == 0)
        return I2C_ERR_24XX_BAD_TYPE;

    if(ctrl_val)
        *ctrl_val = ctrl;
    if(addr_hi_val)
        *addr_hi_val = addr_hi;
    if(addr_lo_val)
        *addr_lo_val = addr_lo;

    return I2C_NO_ERR;
}

//----------------------------------------------------------------------------
static int i2c_lpc_m24xx_wr(
    int eeprom_type,	 //-- EEPROM type
    int eeprom_addr,	 //-- start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,  //-- Hardware A2,A1,A0 (valid from 24XX32)
    unsigned char *buf,		 //-- Data srs buf
    int num )			  //-- Bytes to write qty
{
    int rc;
    int ctrl;
    int addr_hi;
    int addr_lo;

    rc = m24xx_set_addr(eeprom_type,eeprom_addr,eeprom_cs_val,&ctrl,&addr_hi,&addr_lo);

    if(rc != I2C_NO_ERR)
        return rc;

    //--- wr START + CONTROL
    rc = i2c_lpc_ctrl(ctrl); //-- Now WR (RD/WI = 0)
    if(rc != I2C_NO_ERR)
        return rc;
    //--- wr ADDRESS
    i2c_lpc_wr_byte(addr_hi);
    if(addr_lo != -1)
        i2c_lpc_wr_byte(addr_lo);
    //---  Write  data
    while(num--) 				//-- transmit data until length>0
    {
        rc = *buf++; //---
        i2c_lpc_wr_byte(rc);
    }
    //-----------------------
    i2c_lpc_stop();

    rc = i2c_lpc_ask_polling_op(ctrl);	 //-- wait until write finished
    i2c_lpc_stop();
    return I2C_NO_ERR;
}

//----------------------------------------------------------------------------
int m24xx_write(
    int eeprom_type,	//-- EEPROM type
    int eeprom_addr,	//-- start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,	//-- Hardware A2,A1,A0 (valid from 24XX32)
    unsigned char *buf, 		//-- Data src buf
    int num)			//-- Bytes to write qty
{
    int page_size = 0;
    int rc;
    int b_to_wr;



    rc = I2C_NO_ERR;
    for(;;)
    {
        rc = m24xx_page_size(eeprom_type,eeprom_addr,&page_size);
        if(rc != I2C_NO_ERR)
            break;
        if(page_size == 0)
        {
            rc = I2C_ERR_24XX_BAD_PAGESIZE;
            break;
        }

        rc = eeprom_addr%page_size;
        if(rc != 0) //-- not fit on page alignment
        {
            b_to_wr = page_size - rc;
            if(num < b_to_wr)
                b_to_wr = num;
            if(b_to_wr > 0)
            {
                rc = i2c_lpc_m24xx_wr(eeprom_type,eeprom_addr,eeprom_cs_val, buf,b_to_wr);
                if(rc != I2C_NO_ERR)
                    break;
                num -= b_to_wr;
                eeprom_addr += b_to_wr;
                buf += b_to_wr;
            }
        }
        //--- write remainder(by pages,if possible)
        while(num > 0)
        {
            if(num < page_size)
                b_to_wr = num;
            else
                b_to_wr = page_size;

            rc = i2c_lpc_m24xx_wr(eeprom_type,eeprom_addr,eeprom_cs_val, buf,b_to_wr);
            if(rc != I2C_NO_ERR)
                break;
            num -= b_to_wr;
            eeprom_addr += b_to_wr;
            buf += b_to_wr;
        }

        break;
    }



    return rc;
}

//----------------------------------------------------------------------------
int m24xx_read(
    int eeprom_type,	  //-- EEPROM type
    int eeprom_addr,	  //-- start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,  //-- Hardware A2,A1,A0 (valid from 24XX32)
    unsigned char *buf,		  //-- Data dst buf
    int num)			  //-- Bytes to read qty
{
    int page_size;
    int rc;
    int ctrl;
    int addr_hi;
    int addr_lo;



    rc = I2C_NO_ERR;
    for(;;)
    {
        if(num <=0)
        {
            rc = I2C_ERR_24XX_WRONG_NUM;
            break;
        }
        //--- Here - just for addr checking
        page_size = 0;
        rc = m24xx_page_size(eeprom_type,eeprom_addr,&page_size);
        if(rc != I2C_NO_ERR)
            break;
        if(page_size == 0)
        {
            rc = I2C_ERR_24XX_BAD_PAGESIZE;
            break;
        }
        rc = m24xx_set_addr(eeprom_type,eeprom_addr,eeprom_cs_val,&ctrl,&addr_hi,&addr_lo);

        if(rc != I2C_NO_ERR)
            break;

        //--- wr START + CONTROL
        rc = i2c_lpc_ctrl(ctrl & 0xFE); //-- Now WR (RD/WI = 0)
        if(rc != I2C_NO_ERR)
            break;
        //--- wr ADDRESS
        i2c_lpc_wr_byte(addr_hi);
        if(addr_lo != -1)
            i2c_lpc_wr_byte(addr_lo);

        //--- wr START + CONTROL again - for read start
        rc = i2c_lpc_ctrl(ctrl | 0x01); //-- Now RD (RD/WI = 1)
        if(rc != I2C_NO_ERR)
            break;

        rc = i2c_lpc_rx_to_buf(buf,num);
        if(rc != I2C_NO_ERR)
            break;

        i2c_lpc_stop();	  //---- Set STOP ---

        break;
    }


    return rc;
}










