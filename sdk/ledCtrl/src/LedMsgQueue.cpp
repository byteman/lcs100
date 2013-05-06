#include "LedMsgQueue.h"
#include "Poco/SingletonHolder.h"
#define PROTO_HEAD      0xA3
#define PROTO_ACK_PAD   10

LedMessage::LedMessage()
{
    id = 0;
    group  = 0;
    cmd    = 0;
    timeout= 0;
    isRespMsg = false;
    paramLen = 0;
    memset(param,0,MAX_DATA_SIZE);
    evt.reset ();
}
LedMessage::LedMessage(unsigned int _id,unsigned char _group,unsigned char _cmd,bool _needResp,int _respSize,long _timeout)
{
    id = _id;
    group  = _group;
    cmd    = _cmd;
    timeout = _timeout;
    needResp = _needResp;
    isRespMsg = false;
    respSize = _respSize + 10;
    paramLen = 0;
    memset(param,0,MAX_DATA_SIZE);
    evt.reset ();
}

unsigned char LedMessage::checkSum(unsigned char* buff, int size)
{
    unsigned char sum = 0;
    for(int i = 0; i < size; i++)
        sum+=buff[i];
    return sum;
}

bool LedMessage::buildMessage(unsigned char* pktBuff,int pktSize)
{
    if(pktBuff[0] != PROTO_HEAD) return false;

    if(checkSum(pktBuff,pktSize-1) != pktBuff[pktSize-1]) return false;


    int len = pktBuff[1];

    int  parmLen = 0;
    id     = (pktBuff[2]<<24) + (pktBuff[3]<<16) +(pktBuff[4]<<8) +(pktBuff[5]<<0);
    group  = pktBuff[6];
    cmd    = pktBuff[7]&0x3F;
    isRespMsg = ((pktBuff[7]>>6)&0x3)?true:false;

    if(isRespMsg)
    {
        respCode = pktBuff[8];
        parmLen  = len - PROTO_ACK_PAD;
        if(paramLen < 0) return false;
        memcpy(param,pktBuff+9,parmLen);

    }
    else
    {
        parmLen = len - PROTO_ACK_PAD + 1;
        if(paramLen < 0) return false;

        memcpy(param,pktBuff+8,parmLen);
    }

    paramLen = parmLen;

    return true;

}
LedMessage::LedMessage(const LedMessage& msg)
{

    memcpy(param,msg.param,MAX_DATA_SIZE);
}
bool LedMessage::wait(long waitMs)
{
    return evt.tryWait (waitMs);
}
void LedMessage::complete(void)
{
    evt.set ();
}
int LedMessage::getIntVal()
{
    return (param[0]<<24) + (param[1]<<16) + (param[2]<<8) + (param[3]<<0);
}
char LedMessage::getCharVal()
{
    return (param[0]<<0);
}
short LedMessage::getShortVal()
{
    return (param[0]<<8) + (param[1]<<0);
}
float LedMessage::getFloatVal()
{
    return 0;
}
unsigned char*    LedMessage::getBuffVal(int& len)
{
    len = paramLen;
    return param;
}
bool    LedMessage::setBuffVal(unsigned char* buff, int bufSize)
{
    if(bufSize > MAX_DATA_SIZE) return false;

    memcpy(param,buff,bufSize);
    paramLen = bufSize;
    return true;
}
void     LedMessage::setIntVal(int val)
{
    param[0] = val>>24;
    param[1] = val>>16;
    param[2] = val>>8;
    param[3] = val>>0;
    paramLen = 4;
}
void    LedMessage::setCharVal(char val)
{
    param[0] = val>>0;
    paramLen = 1;
}
void   LedMessage::setShortVal(short val)
{
    param[0] = val>>8;
    param[1] = val>>0;
    paramLen = 2;
}
void   LedMessage::setFloatVal(float val)
{

}

LedMsgQueue::LedMsgQueue()
{
}

void LedMsgQueue::push(LedMessage msg)
{
    _msgQue.push (msg);
}
LedMessage LedMsgQueue::front()
{
    return _msgQue.front ();
}
void LedMsgQueue::pop()
{
    _msgQue.pop ();
}
bool LedMsgQueue::empty()
{
    return _msgQue.empty ();
}
