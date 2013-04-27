#ifndef LEDUPLOAD_H
#define LEDUPLOAD_H

#include "Poco/Thread.h"
#include "Poco/Timer.h"
#include <vector>
//#include "SerialPort.h"
#include "serial/serial.h"
#include "LedCtrl.h"
#include <queue>



class LedUpload:public Poco::Runnable
{
public:
    enum UploadState
    {
        STATE_IDLE=0,
        STATE_REQ,
        STATE_DATA,
        STATE_VERIFY,
        STATE_OK
    };
    typedef std::queue<unsigned int> DeviceIDList;
    LedUpload();
    static  LedUpload& get();
    bool    init(std::string uartPath);
    bool    setPort(serial::Serial* comPort);

    bool    loadUploadFile(std::string fileName);

    bool    startUploadFile(std::vector<unsigned int>& devList);

	bool	startUploadGroupFile(unsigned group);
	bool	uploadHasComplete(void);
	bool	uploadProc(void);
    int     getPacketNum()
    {
        return _packetNum;
    }
private:
	
    void    simulateUpload(void);
    void    notify(LedEvent event,unsigned int id, LedError err);
    bool    sendUploadRequest();
    bool    sendUploadData();
    bool    sendUploadVerify();
    void    timerOut(Poco::Timer& timer);
    void    run();
    void    runStateMachine();
    bool    sendReset();
    bool    sendPacket(unsigned char* context, int len);

    bool    parsePacket(unsigned char* context, int len);
    void    parseUploadVerifyResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid);
    void    parseUploadReqResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid);
    void    parseUploadDataResponse(Poco::UInt32 id, Poco::UInt8 grp,unsigned char ack,unsigned short ssid, unsigned short pktIdx);
    void dumpData(unsigned char* buff, int len);
    unsigned short getSessionID();
    void resetState();
    unsigned char *pUploadFile;
    unsigned short _packetNum;
    unsigned short _packetSize;
    unsigned short _packetIdx;
    unsigned int   _totalFileSize;
    unsigned short _fileCRC16;
    unsigned int   _targetID;
    unsigned short _curSessionID;
    UploadState    _state;
	unsigned int	_timeout;
    Poco::Timer    _timer;

    Poco::Thread   _thread;
    Poco::Event    _evtRdy;
    bool           _quit;
    unsigned char context[512];
    unsigned char packet[512];

	DeviceIDList		_deviceList;
	Poco::Event		_evtUpload;
	bool	_complete;
};

#endif // LEDUPLOAD_H
