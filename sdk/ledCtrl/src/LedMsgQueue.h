#ifndef LEDMSGQUEUE_H
#define LEDMSGQUEUE_H

#include <string.h>
#include <queue>
#include "Poco/Event.h"
#define MAX_DATA_SIZE 64


struct LedMessage
{
    void complete(void);
    bool wait(long waitMs);
    int  buildPacket();
    unsigned char checkSum(unsigned char* buff, int size);
    LedMessage(const LedMessage& msg);
    LedMessage();
    LedMessage(unsigned int _id,unsigned char _group,unsigned char _cmd,bool _needResp=false,int _respSize=0,long _timeout=1000);
    bool    buildMessage(unsigned char* pktBuff,int pktSize);
	bool	buildMessageFromContext(unsigned char* pktBuff,int pktSize);
    int     getIntVal();
    char    getCharVal();
    short   getShortVal();
    float   getFloatVal();
    unsigned char*    getBuffVal(int& len);

    void    setIntVal(int val);
    void    setCharVal(char val);
    void    setShortVal(short val);
    void    setFloatVal(float val);
    bool    setBuffVal(unsigned char* buff, int bufSize);

    unsigned int  id;
    unsigned char group;
    unsigned char cmd;
    long timeout;

    unsigned char param[MAX_DATA_SIZE];
    int           paramLen;
    int           respSize;
    bool          needResp;
    bool          isRespMsg;
    unsigned char respCode;

    Poco::Event   evt;
};

typedef std::queue<LedMessage> TLedMsgQue;
class LedMsgQueue
{
public:
    LedMsgQueue();
    void push(LedMessage msg);
    LedMessage front();
    void pop();
    bool empty();
private:
    TLedMsgQue _msgQue;

};

#endif // LEDMSGQUEUE_H
