#include "ByProtocol.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


#define PAD_SIZE 3
#define MAX_CONTEXT_SIZE (0xFF-PAD_SIZE)
#define KEY_HEADER	0xA3

#ifdef __C51__
#define VAR_ATTR xdata
#else
#define VAR_ATTR
#endif

#undef LENGTH_CHECK
/*
一种基于消息头的不定长度的协议解析函数,用于总数据长度小于255个字节
头部		: 0xA3
数据总长度  : 1 Bytes
长度的补码  : 1 Bytes (为了效率，可以不用)
数据内容    : n Bytes (不定长度,内容体可以自定义)
校验和	    : 1 Bytes(前面所有数据包括头部的和校验)
*/
typedef enum
{
    STEP_HEADER=0,
    STEP_LENGTH,
#ifdef LENGTH_CHECK
    STEP_CHECK_LENGHT,
#endif
    STEP_DATA,
    STEP_CRC,
} StepParser;


static VAR_ATTR unsigned char totalBytes;
static VAR_ATTR unsigned char context[MAX_CONTEXT_SIZE];

static VAR_ATTR unsigned char prevContext[MAX_CONTEXT_SIZE];
static VAR_ATTR unsigned char prevContextBytes = 0;

static VAR_ATTR unsigned char checkSum = 0;
static VAR_ATTR unsigned char curStep = 0;
static VAR_ATTR unsigned char curPos = 0;
static VAR_ATTR unsigned char contextBytes = 0;
static VAR_ATTR unsigned char packetValid= 0; //接收到一个数据包之后变成有效，接收到新的头部后，变成无效
static VAR_ATTR DataProcType  dataProcFunc = NULL;


static void parserInit(unsigned char step);


void protoParserInit(DataProcType cbProc)
{
    dataProcFunc = cbProc;
    parserInit(STEP_HEADER);
}

void parserInit(unsigned char step)
{

    curStep	 = step;
    curPos   = 0;
    checkSum = 0;
}

unsigned char checkBufSum(unsigned char* buf, unsigned int len)
{
    VAR_ATTR unsigned char sum = 0;
    VAR_ATTR unsigned int  i   = 0;
    for(; i < len; i++)
    {
        sum += buf[i];

    }
    return sum;
}
unsigned int  buildPacket(unsigned char* context, unsigned int contextSize,
                          unsigned char* packet, unsigned int pktSize)
{
    VAR_ATTR unsigned int i = 0;

    assert(context && packet);
    assert (pktSize > (contextSize+PAD_SIZE));

    packet[i++] = KEY_HEADER;
    packet[i++] = contextSize+PAD_SIZE;
#ifdef LENGTH_CHECK
    pakcet[i++] = (~pakcet[i-1]) + 1;
#endif
    memcpy(packet+i,context,contextSize);
    i += contextSize;
    packet[i++] = checkBufSum(packet,i);

    return i;
}
unsigned char* readPacket(unsigned int* pktLen)
{
    if(packetValid==0) return NULL;

    *pktLen = prevContextBytes;
    //packetValid = 0;
    return prevContext;
}
unsigned char parseChar(unsigned char rxChar)
{
    VAR_ATTR unsigned char ret = 0;
    switch(curStep)
    {
    case STEP_HEADER:
        if(rxChar == KEY_HEADER)
        {
            context[curStep++] = rxChar;
            curPos++;
            packetValid = 0;
        }
        break;
    case STEP_LENGTH:
        totalBytes	 = rxChar;
        if(totalBytes > PAD_SIZE)
        {
            contextBytes = totalBytes-PAD_SIZE;
            context[curStep++] = rxChar;
            curPos++;
        }
        else
        {
            parserInit(STEP_HEADER);
        }

        break;
#ifdef LENGTH_CHECK
    case STEP_CHECK_LENGHT:
        if( ((~rxChar)+1) == totalBytes)
        {
            context[curStep++] = rxChar;
            curPos++;
        }
        else //长度校验不通过，无效数据，从新开始接收
        {
            parserInit(STEP_HEADER);
        }
        break;
#endif
    case STEP_DATA:
        if(curPos < totalBytes-1)
        {
            context[curPos] = rxChar;
            curPos++;
            if( curPos == totalBytes-1 )
            {
                curStep++;
            }
        }
        else
        {
            parserInit(STEP_HEADER);
        }
        break;
    case STEP_CRC:
        if(checkBufSum(context,totalBytes-1) == rxChar)
            //if(1)
        {
            context[curPos] = rxChar;

            //拷贝接收到的数据区中的数据到已接收数据队列中去,这里只能保存最近的一个数据包，可以考虑保存多个数据包
            memcpy(prevContext, context+STEP_DATA,contextBytes);
            prevContextBytes = contextBytes;

            if(dataProcFunc)
            {
                dataProcFunc(context+STEP_DATA,contextBytes);
            }
            parserInit(STEP_HEADER);
            ret = 1;
            packetValid = 1;

        }
        else
        {
            parserInit(STEP_HEADER);
        }
        break;
    default:
        parserInit(STEP_HEADER);
        break;
    }

    return ret;

}