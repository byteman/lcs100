/******  重庆星河光电   ***********/


#include "LPC11xx.h"
#include "uart.h"
#include "Type.h"
#include "string.h"
#include "tinyfifo.h"
volatile uint32_t UARTStatus;
volatile uint8_t  UARTTxEmpty = 1;
uint8_t  UARTBuffer[BUFSIZE];
volatile uint32_t UARTCount = 0;
//uint8_t  Flag_Setup =FALSE;

uint8_t  TempBuffer[BUFSIZE];

uint8_t  rxChar = 0;

uint32_t UARTRxLength = 0;
uint8_t  Flag_Uart_Rx =FALSE;

uint8_t Flag = 0;



/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
**
*****************************************************************************/
/*
#pragma diag_suppress=Pa082
当一个语句中有2个以上的volatile修饰的变量的时候，编译器会出这个警告，编译器无法安排多个volatile变量的访问顺序。
寄存器定义的时候都是有volatile修饰的。
*/

void UART_IRQHandler(void)
{

    while ((LPC_UART->IIR & 0x01) == 0)
    {
        while ((LPC_UART->LSR & 0x01) == 0x01)
        {
            rxChar = LPC_UART->RBR;
            tinyFifoPutc(rxChar);
        }

    }

}

#if 0
void ModemInit( void )
{
    LPC_IOCON->PIO2_0 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO2_0 |= 0x01;     /* UART DTR */
    LPC_IOCON->PIO0_7 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO0_7 |= 0x01;     /* UART CTS */
    LPC_IOCON->PIO1_5 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO1_5 |= 0x01;     /* UART RTS */
#if 1
    LPC_IOCON->DSR_LOC	= 0;
    LPC_IOCON->PIO2_1 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO2_1 |= 0x01;     /* UART DSR */

    LPC_IOCON->DCD_LOC	= 0;
    LPC_IOCON->PIO2_2 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO2_2 |= 0x01;     /* UART DCD */

    LPC_IOCON->RI_LOC	= 0;
    LPC_IOCON->PIO2_3 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO2_3 |= 0x01;     /* UART RI */

#else
    LPC_IOCON->DSR_LOC = 1;
    LPC_IOCON->PIO3_1 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO3_1 |= 0x01;     /* UART DSR */

    LPC_IOCON->DCD_LOC = 1;
    LPC_IOCON->PIO3_2 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO3_2 |= 0x01;     /* UART DCD */

    LPC_IOCON->RI_LOC = 1;
    LPC_IOCON->PIO3_3 &= ~0x07;    /* UART I/O config */
    LPC_IOCON->PIO3_3 |= 0x01;     /* UART RI */
#endif
    LPC_UART->MCR = 0xC0;          /* Enable Auto RTS and Auto CTS. */
    return;
}
#endif



void UARTInit(uint32_t baudrate)
{
    uint32_t Fdiv;
    uint32_t regVal;

    UARTTxEmpty = 1;
    UARTCount = 0;
    Flag_Uart_Rx = 0;

    NVIC_DisableIRQ(UART_IRQn);

    LPC_IOCON->PIO1_6 &= ~0x07;	 /*  UART I/O config */
    LPC_IOCON->PIO1_6 |= 0x01;	 /* UART RXD */
    LPC_IOCON->PIO1_7 &= ~0x07;
    LPC_IOCON->PIO1_7 |= 0x01;	 /* UART TXD */

    /* Enable UART clock 使能UART时钟之前必须将UART管脚配置好*/
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
    LPC_SYSCON->UARTCLKDIV = 0x1; 	/* divided by 1 */

    LPC_UART->LCR = 0x83; 			/* 8 bits, no Parity, 1 Stop bit */
    regVal = LPC_SYSCON->UARTCLKDIV;
    Fdiv = ((SystemAHBFrequency/regVal)/16)/baudrate ;	/*baud rate */

    LPC_UART->DLM = Fdiv / 256;
    LPC_UART->DLL = Fdiv % 256;
    LPC_UART->LCR = 0x03; 	/* DLAB = 0 */
    LPC_UART->FCR = 0x87; 	/* Enable and reset TX and RX FIFO. */

    /* Read to clear the line status. */
    regVal = LPC_UART->LSR;

    /* Ensure a clean start, no data in either TX or RX FIFO. */
    while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
    while ( LPC_UART->LSR & LSR_RDR )
    {
        regVal = LPC_UART->RBR; /* Dump data from RX FIFO */
    }

    /* Enable the UART Interrupt */
    NVIC_EnableIRQ(UART_IRQn);

#if TX_INTERRUPT
    LPC_UART->IER = IER_RBR | IER_THRE | IER_RLS; /* Enable UART interrupt */
#else
    LPC_UART->IER = IER_RBR | IER_RLS;	/* Enable UART interrupt */
#endif
    return;
}


void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{

    while ( Length != 0 )
    {
        /* THRE status, contain valid data */
#if !TX_INTERRUPT
        while ( !(LPC_UART->LSR & LSR_THRE) );
        LPC_UART->THR = *BufferPtr;
#else
        /* Below flag is set inside the interrupt handler when THRE occurs. */
        while ( !(UARTTxEmpty & 0x01) );
        LPC_UART->THR = *BufferPtr;
        UARTTxEmpty = 0;	/* not empty in the THR until it shifts out */
#endif
        BufferPtr++;
        Length--;
    }
    return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
