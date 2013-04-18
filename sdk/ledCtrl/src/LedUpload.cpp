#include "LedUpload.h"
#include <Poco/SingletonHolder.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <string.h>
#include <assert.h>
#include <cstdio>
#include "ByProtocol.h"
//#include "SerialPort.h"
#include "crc.h"
using Poco::AbstractTimerCallback;

#define MAX_PACKET_SIZE 64

//static LibSerial::SerialPort*    _zigbeeCom = NULL;
static serial::Serial*    _zigbeeCom = NULL;
static unsigned short gSessionID= 1;

LedUpload::LedUpload():
    pUploadFile(NULL),
    _quit(false)
{
    _packetSize = MAX_PACKET_SIZE;
    _packetIdx  = 0;
    _totalFileSize = 0;
    _state = STATE_IDLE;

}
bool    LedUpload::init(std::string uartPath)
{
	return true;
}
bool    LedUpload::setPort(serial::Serial* comPort)
{
    _zigbeeCom = comPort;
    return true;
}

LedUpload& LedUpload::get()
{
    static Poco::SingletonHolder<LedUpload> sh;
    return *sh.get ();
}

bool    LedUpload::loadUploadFile(std::string fileName)
{
    try
    {
        Poco::File upFile(fileName);
        int sz = upFile.getSize ();
        if(sz <= 1024)  //upload file too small
        {
            fprintf(stderr,"[%s]:upload file too small \n",fileName.c_str ());
            return false;
        }

        _packetNum = (sz + _packetSize-1) /_packetSize;
        _totalFileSize = _packetNum*_packetSize;
        //_packetNum = 4096*7 / 64;
        //_totalFileSize = 4096*7 ;
        if(pUploadFile)
        {
            delete pUploadFile;
            pUploadFile = NULL;
        }
        pUploadFile = new unsigned char[_totalFileSize];
        memset(pUploadFile,0,_totalFileSize);

        Poco::FileInputStream ss(fileName);
        ss.read ((char*)pUploadFile,sz);

        _fileCRC16 = u16CRC_Calc16(pUploadFile,_totalFileSize);

        fprintf(stderr,"totalsize=%d,pktsize=%d,ptknum=%d,crc=%02x\n",_totalFileSize,_packetSize,_packetNum,_fileCRC16);
        return true;
    }
    catch(Poco::FileNotFoundException& e)
    {
        fprintf(stderr,"can't find %s\n",fileName.c_str ());
    }
    catch(...)
    {

    }
    if(pUploadFile) delete []pUploadFile;
    return false;



    return true;
}

unsigned short LedUpload::getSessionID()
{
    return gSessionID++;
}
/**/
void LedUpload::dumpData(unsigned char* buff, int len)
{
    for(int i = 0; i < len; i++)
    {
        printf("0x%02x ",buff[i]);
    }
    printf("\n");
}
bool    LedUpload::sendPacket(unsigned char* context, int len)
{
    int sendlen = buildPacket(context,len,packet,512);
    dumpData(packet,sendlen);
    _zigbeeCom->write (packet,sendlen);

	return true;
}

void    LedUpload::timerOut(Poco::Timer& timer)
{
    fprintf(stderr,"calltimer\n");
    runStateMachine();
}
void    LedUpload::parseUploadReqResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid)
{
    if(ssid != _curSessionID)
    {
        TEventParam par(id,grp,EV_REQ,ERR_SESSION);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"session ID err: [%d][%d]\n",ssid,_curSessionID);
        //_state = STATE_IDLE;

        return;
    }
    if(ack == 0)
    {
        _packetIdx = 0;
        _state = STATE_DATA;
        TEventParam par(id,grp,EV_REQ,ERR_OK);
        LedCtrl::get ().notify (&par);
    }
    else
    {
        TEventParam par(id,grp,EV_REQ,(LedError)ack,(LedError)ack);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"err:ack=%d\n",ack);
    }

}

void   LedUpload::parseUploadVerifyResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid)
{
    if(ssid != _curSessionID)
    {
        TEventParam par(id,grp,EV_REQ,ERR_SESSION);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"session ID err: [%d][%d]\n",ssid,_curSessionID);
        //_state = STATE_IDLE;
        return;
    }
    if(ack == 0)
    {
        TEventParam par(id,grp,EV_VERIFY,ERR_OK);
        LedCtrl::get ().notify (&par);
        _state = STATE_OK;
    }
    else
    {
        TEventParam par(id,grp,EV_VERIFY,(LedError)ack,(LedError)ack);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"err:ack=%d\n",ack);
    }
}

void    LedUpload::parseUploadDataResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid, unsigned short pktIdx)
{
    if(ssid != _curSessionID)
    {
        TEventParam par(id,grp,EV_DATA,ERR_SESSION);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"session ID err: [%d][%d]\n",ssid,_curSessionID);
        //_state = STATE_IDLE;
        return;
    }
    if(ack == 0)
    {
        printf("packet[%d] send ok\n",pktIdx);
        if(pktIdx != _packetIdx)
        {
            fprintf(stderr,"error index[%d][%d]\n",pktIdx,_packetIdx);
        }
        _packetIdx++;
        if(_packetIdx >= _packetNum)
        {
            _state = STATE_VERIFY;
        }

        TEventParam par(id,grp,EV_DATA,(LedError)ack,_packetIdx);
        LedCtrl::get ().notify (&par);
    }
    else
    {
        TEventParam par(id,grp,EV_DATA,(LedError)ack,(LedError)ack);
        LedCtrl::get ().notify (&par);
        fprintf(stderr,"err:ack=%d\n",ack);
    }
}

bool    LedUpload::parsePacket(unsigned char* context, int len)
{
    Poco::UInt32 termID  = ( ( context[0]<<24) + ( context[1]<<16) + ( context[2]<<8) +( context[3]<<0));
    Poco::UInt8  groupID = context[4];
    Poco::UInt8  code    = context[5] & 0x3F;
    if(termID != _targetID)
    {
        return false;
    }

    switch(code)
    {
        case CMD_UPLOAD_REQ:
            parseUploadReqResponse(termID,groupID,context[6],(context[7]<<8) + context[8]);
            break;
        case CMD_UPLOAD_DATA:
            parseUploadDataResponse(termID,groupID,context[6],(context[7]<<8) + context[8],(context[9]<<8) + context[10]);
            break;
        case CMD_UPLOAD_VERIFY:
            parseUploadVerifyResponse(termID,groupID,context[6],(context[7]<<8) + context[8]);
            break;
        default:
            break;
    }
    return true;
}
void    LedUpload::run()
{
    _evtRdy.set ();
    _quit = false;
    runStateMachine();
	_zigbeeCom->setReadTimeout(1000);
    while(!_quit && (_state != STATE_OK))
    {

		try
		{
			unsigned char ch = 0;

			if(_zigbeeCom->read(&ch,1) > 0)
			{
				if( parseChar( ch ))
				{
					unsigned int pktLen = 0;
					unsigned char * pkt = readPacket(&pktLen);
					if(pkt)
					{
						parsePacket(pkt,pktLen);
						runStateMachine();
					}
				}
			}
			else
			{
				runStateMachine();
			}
		}
		catch(serial::PortNotOpenedException& e)
		{
			fprintf(stderr,"%s\r\n",e.what());
			_quit = true;
			//runStateMachine();
		}
		catch(serial::IOException& e)
		{
			fprintf(stderr,"IOException\r\n");
			_quit = true;
		}
		catch(...)
		{
			fprintf(stderr,"Unknown err\r\n");
			_quit = true;
		}

    }
    resetState ();
}
bool    LedUpload::sendUploadRequest()
{

    context[0] = _targetID>>24;
    context[1] = _targetID>>16;
    context[2] = _targetID>>8;
    context[3] = _targetID>>0;
    context[4] = 0; //group
    context[5] = CMD_UPLOAD_REQ;

    context[6] = _curSessionID>>8;
    context[7] = _curSessionID>>0;

    context[8] = _totalFileSize>>24;
    context[9] = _totalFileSize>>16;
    context[10] = _totalFileSize>>8;
    context[11] = _totalFileSize>>0;

    sendPacket(context,12);

	return true;
    //Poco::Thread::sleep (1000);

}
bool    LedUpload::sendUploadVerify()
{
    context[0] = _targetID>>24;
    context[1] = _targetID>>16;
    context[2] = _targetID>>8;
    context[3] = _targetID>>0;
    context[4] = 0; //group
    context[5] = CMD_UPLOAD_VERIFY;
    context[6] = _curSessionID>>8;
    context[7] = _curSessionID>>0;
    context[8] = _fileCRC16>>8;
    context[9] = _fileCRC16>>0;
    sendPacket (context,10);
    //Poco::Thread::sleep (1000);

	return true;
}
bool    LedUpload::sendUploadData()
{
    if(_packetIdx < _packetNum)
    {
        context[0] = _targetID>>24;
        context[1] = _targetID>>16;
        context[2] = _targetID>>8;
        context[3] = _targetID>>0;
        context[4] = 0; //group
        context[5] = CMD_UPLOAD_DATA;
        context[6] = _curSessionID>>8;
        context[7] = _curSessionID>>0;
        context[8] = _packetIdx>>8;
        context[9] = _packetIdx&0xFF;

        int offset = _packetIdx*_packetSize;
        int size   = _packetSize;
        memcpy(context+10,pUploadFile+offset,size);

        sendPacket (context,10+size);
        //Poco::Thread::sleep (1000);
        return true;
    }
    else
    {
        fprintf(stderr,"_packetIdx too large\n");
    }
    return false;
}
bool    LedUpload::sendReset()
{
    context[0] = _targetID>>24;
    context[1] = _targetID>>16;
    context[2] = _targetID>>8;
    context[3] = _targetID>>0;
    context[4] = 0; //group
    context[5] = CMD_RESET;
    sendPacket (context,6);

	return true;
}
void    LedUpload::runStateMachine()
{
    printf("state=%d\n",_state);
    switch(_state)
    {
        case STATE_REQ:
            sendUploadRequest();
            break;
        case STATE_DATA:
            sendUploadData();
            break;
        case STATE_VERIFY:
            sendUploadVerify ();
            break;
        case STATE_OK:
            sendReset ();
            break;
    }
}
void LedUpload::resetState()
{
    _state = STATE_IDLE;
    _packetIdx = 0;
    _targetID = 0;
}
bool    LedUpload::startUploadFile(unsigned int devID)
{
    if(_state != STATE_IDLE)
    {
        return false;
    }
    _state = STATE_REQ;
    _targetID = devID;
    _curSessionID = getSessionID();
#if 0
    Poco::TimerCallback<LedUpload> callback(*this,&LedUpload::timerOut);
    _timer.setStartInterval (10);
    _timer.setPeriodicInterval (1000);
    _timer.start(callback);
#endif
    _thread.start (*this);
    if(_evtRdy.tryWait (1000)==false) return false;

    protoParserInit(NULL);
    //等待30秒升级线程结束
    //这里Poco库有一个bug，需要修改
    // /media/linuxdata/home/byteman/library/poco/Foundation/src/Thread_POSIX.cpp中 304 line
    if(false == _thread.tryJoin (50000))
    {
        _quit = true;
        _thread.join ();

        return false;
    }
    return true;

}
bool    LedUpload::startUploadFile(DeviceIDList& devList)
{
    for(size_t i = 0 ;i < devList.size ();i++)
    {
        if(!startUploadFile(devList.at(i)))
        {
            fprintf(stderr,"%d upload failed\n",devList.at(i));
        }
    }
    return true;
}
