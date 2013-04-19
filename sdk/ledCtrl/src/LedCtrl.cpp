#include "LedCtrl.h"
#include "LedUpload.h"
#include "ByProtocol.h"
#include "LedMsgQueue.h"
#include "serial/serial.h"
#include <Poco/SingletonHolder.h>
#include <Poco/ThreadPool.h>
#include <stdexcept>
#include <map>
typedef std::map<unsigned int,StreetLight> TStreeLightList;
#define PROTO_HEAD 0xA3
#define PROTO_ACK_PAD 10
static serial::Serial *pZigbeeCom = NULL;
//static LibSerial::SerialPort* _zigbeeCom = NULL;

static LedMsgQueue syncMsgQue;
static LedMsgQueue asyncMsgQue;
static TZigbeeCfg gZigbeeCfg;
static ILedEventNofityer* gEventNotifyer = NULL;


static unsigned char context[1024];
static unsigned char param[1024];
class LedUploadThread:public Poco::Runnable
{
public:
    LedUploadThread():
        _complete(true)
    {

    }

    void set(DeviceList devlist)
    {
        _devlist = devlist;
    }
    void run()
    {
        LedUpload::get ().startUploadFile (_devlist);
        _complete = true;
    }
    bool hasComplete()
    {
        return _complete;
    }
private:
    std::string _file;
    DeviceList _devlist;
    bool _complete;
};

static LedUploadThread uploader;

class LedDispatcher:public Poco::Runnable
{
public:
    LedDispatcher():
        _quit(false)
    {

    }
    bool    parsePacket(unsigned char* context, int len)
    {
        Poco::UInt32 termID  = ( ( context[0]<<24) + ( context[1]<<16) + ( context[2]<<8) +( context[3]<<0));
        Poco::UInt8  groupID = context[4];
        Poco::UInt8  code    = context[5] & 0x3F;


        return true;
    }
    void run()
    {
        _evtRdy.set ();
        _quit = false;
        //runStateMachine();
        while(!_quit)
        {
            try
            {
                unsigned char ch = 0;

                if(pZigbeeCom->read(&ch,1) > 0)
                {
                    if( parseChar( ch ))
                    {
                        unsigned int pktLen = 0;
                        unsigned char * pkt = readPacket(&pktLen);
                        if(pkt)
                        {
                            parsePacket(pkt,pktLen);
                            //runStateMachine();
                        }
                    }
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
                fprintf(stderr,"Unkown err\r\n");
                _quit = true;
            }

        }
        //resetState ();

    }
private:
    bool _quit;
    Poco::Event _evtRdy;
};
LedCtrl::LedCtrl()
{


}
void LedCtrl::notify(TEventParam* par)
{
    if(gEventNotifyer)
    {
        gEventNotifyer->notify(par);
    }
}
LedCtrl& LedCtrl::get()
{
    static Poco::SingletonHolder<LedCtrl> sh;
    return *sh.get ();
}
bool LedCtrl::open(std::string comPath,unsigned int bps)
{
#if 1

    try
    {
        if(pZigbeeCom)
        {
            pZigbeeCom->close ();
            delete pZigbeeCom;
            pZigbeeCom = NULL;
        }
        if(pZigbeeCom == NULL)
            pZigbeeCom = new serial::Serial(comPath,bps);
        //pZigbeeCom->open ();
        return true;
    }
    catch(std::invalid_argument& e)
    {
        //fprintf(stderr,"open:%s\r\n",e.what());
        return false;
    }
    catch(serial::SerialExecption& e)
    {
        //fprintf(stderr,"open err\r\n");
        return false;
    }
    catch(serial::IOException& e)
    {
        //fprintf(stderr,"open:%s\r\n",e.what());
        return false;
    }
    catch(...)
    {
        fprintf(stderr,"ledCtrl Open failed: unknown err\r\n");
        return false;
    }
#else

    try
    {
        if(_zigbeeCom == NULL)
        {
            _zigbeeCom = new LibSerial::SerialPort(comPath);
        }
        _zigbeeCom->Open (LibSerial::SerialPort::BAUD_19200);
        return true;
    }
    catch(LibSerial::SerialPort::AlreadyOpen& e)
    {
        fprintf(stderr,"already open\n");
    }
    catch(LibSerial::SerialPort::OpenFailed& e)
    {
        fprintf(stderr,"OpenFailed\n");
    }
    return false;
#endif

}
bool LedCtrl::upload(std::string file,DeviceList devlist)
{
    LedUpload::get ().setPort (pZigbeeCom);
    if(!LedUpload::get ().loadUploadFile (file)) return false;
    uploader.set (devlist);

    Poco::ThreadPool::defaultPool ().start (uploader);
}

unsigned char LedCtrl::checkSum(unsigned char* buff, int len)
{
    unsigned char sum = 0;
    for(int i = 0; i < len; i++)
        sum += buff[i];
    return sum;
}

static void dumpData(unsigned char* buff, int len)
{
    for(int i = 0; i < len; i++)
    {
        printf("0x%02x ",buff[i]);
    }
    printf("\n");
}

int LedCtrl::sendMessage(LedMessage* pMsg)
{
    int totalLen = 8;
    //unsigned short crc16 = 0;
    context[0] = PROTO_HEAD;
    context[1] = 0;
    context[2] = (pMsg->id) >>24;
    context[3] = (pMsg->id) >>16;
    context[4] = (pMsg->id) >>8;
    context[5] = (pMsg->id) >>0;
    context[6] = (pMsg->group);
    context[7] = (pMsg->cmd)&0x3F;
    memcpy(context+8,pMsg->param,pMsg->paramLen);
    totalLen += pMsg->paramLen;
    context[1] = totalLen+1;
    context[totalLen]   = checkSum(context,totalLen);
    totalLen++;

    dumpData(context,totalLen);
    return pZigbeeCom->write (context,totalLen);
}

bool LedCtrl::setDeviceReset(unsigned int id,unsigned char group,unsigned int afterMs)
{
    return false;
}
bool LedCtrl::hasUploadComplete(void)
{
    return uploader.hasComplete ();
}
void LedCtrl::addObserver(ILedEventNofityer* obs)
{
    gEventNotifyer = obs;
}
int  LedCtrl::getUploadFilePacketNum(void)
{
    return LedUpload::get().getPacketNum ();
}
bool    parsePacket(LedMessage* pMsg,unsigned char* context, int len)
{
    Poco::UInt32 termID  = ( ( context[0]<<24) + ( context[1]<<16) + ( context[2]<<8) +( context[3]<<0));
    Poco::UInt8  groupID = context[4];
    Poco::UInt8  code    = context[5] & 0x3F;


    if(pMsg->id != termID) return false;
    if(pMsg->cmd != code) return false;

    //拷贝数据部分到pMsg的respParam中，
    return true;
}

bool LedCtrl::checkPacketValid(LedMessage* pReqMsg,LedMessage* pRespMsg)
{
    if(pReqMsg->id  != pRespMsg->id)  return false;
    if(pReqMsg->cmd != pRespMsg->cmd) return false;

    return true;
}

bool LedCtrl::waitRespMessage(LedMessage* pReqMsg,LedMessage* pRespMsg)
{
    unsigned char buf[128];

    if(!pReqMsg->needResp) return true;

    try
    {
        pZigbeeCom->setReadTimeout(pReqMsg->timeout);
        int ret = pZigbeeCom->read (buf,pReqMsg->respSize);

        if(!pRespMsg->buildMessage (buf, ret))
        {
            return false;
        }
        return checkPacketValid(pReqMsg,pRespMsg);
    }
    catch(...)
    {
        fprintf(stderr,"waitPacket timeout\n ");
    }
    return false;
}
int  LedCtrl::getDeviceResetCount(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_GET_RESET_CNT,2,waitMs);
}
int  LedCtrl::setBrigtness(unsigned int id,unsigned char group,unsigned char value,long waitMs)
{
    LedMessage respMsg;
    LedMessage reqMsg(id,group,CMD_ADJUST_BRIGHTNESS,true,0,waitMs);
    reqMsg.setCharVal (value);

    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        if(respMsg.respCode == ERR_OK)
            return value;
    }

    return -1;
}
int  LedCtrl::setIntResp(unsigned int id,unsigned char group,LedCmdType type,int value,int size,long waitMs)
{
    if(!uploader.hasComplete ())
    {
        printf("upload has not complete\n");
        return -1;
    }
    LedMessage respMsg;
    LedMessage reqMsg(id,group,type,true,size,waitMs);

    switch(size)
    {
    case 1:
        reqMsg.setCharVal (value);
        break;
    case 2:
        reqMsg.setShortVal (value);
        break;
    case 4:
        reqMsg.setIntVal (value);
        break;
    default:
        reqMsg.setCharVal (value);
        break;
    }
    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        return -respMsg.respCode;
    }

    return -1;
}
int  LedCtrl::setShakeLed(unsigned int id,unsigned char group,unsigned short value,long waitMs)
{
    return setIntResp(id,group,CMD_TWINKLE,value,2,waitMs);
}
int  LedCtrl::writeE2prom(unsigned int id,unsigned char group,LedParam* pPara,long waitMs)
{
    LedMessage respMsg;
    LedMessage reqMsg(id,group,CMD_WRITE_EEPROM,true,0,waitMs);
    reqMsg.setBuffVal ((unsigned char*)pPara,sizeof(LedParam));

    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        return respMsg.respCode;
    }

    return -1;
}
int  LedCtrl::getAllData(unsigned int id,unsigned char group,StreetLight* pLight,long waitMs)
{
    fprintf(stderr,"not support\n");
    return -1;

}
int  LedCtrl::getCureent(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_CURRENT,2,waitMs);
}
int  LedCtrl::getVoltage(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_VOLTAGE,2,waitMs);
}
int  LedCtrl::getKw(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_KW,2,waitMs);
}
StreetLight*  LedCtrl::getLightParam(unsigned int id)
{
    return NULL;
}

int  LedCtrl::setZigbeeCfg(unsigned int id,TZigbeeCfg* pCfg,long waitMs)
{
    LedMessage respMsg;
    LedMessage reqMsg(id,0,CMD_SET_ZIGBEE_CFG,true,0,waitMs);

    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        if(respMsg.respCode == ERR_OK)
        {
            return 0;
        }
        return -respMsg.respCode;

    }
    return -1;
}

TZigbeeCfg*  LedCtrl::getZigbeeCfg(unsigned int id,long waitMs)
{
    LedMessage respMsg;
    LedMessage reqMsg(id,0,CMD_QUERY_ZIGBEE_CFG,true,sizeof(TZigbeeCfg),waitMs);

    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        if(respMsg.respCode == ERR_OK)
        {
            int len = 0;
            unsigned char* pSrc = respMsg.getBuffVal (len);
            if(len != sizeof(TZigbeeCfg)) return NULL;

            memcpy((unsigned char*)&gZigbeeCfg,pSrc,len);
            return &gZigbeeCfg;
        }

    }

    return NULL;
}
int  LedCtrl::getIntResp(unsigned int id,LedCmdType type,int size,long waitMs)
{
    if(!uploader.hasComplete ())
    {
        printf("upload has not complete\n");
        return -1;
    }
    LedMessage respMsg;
    LedMessage reqMsg(id,0,type,true,size,waitMs);

    sendMessage (&reqMsg);

    if( waitRespMessage (&reqMsg,&respMsg))
    {
        if(respMsg.respCode == ERR_OK)
        {
            switch(size)
            {
            case 1:
                return respMsg.getCharVal ();
                break;
            case 2:
                return respMsg.getShortVal ();
                break;
            case 4:
                return respMsg.getIntVal ();
                break;
            default:
                break;
            }
        }

        return -respMsg.respCode;
    }

    return -1;
}
int  LedCtrl::getVersion(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_VERSION,1,waitMs);
}
int  LedCtrl::setAdjustTime(unsigned int id,unsigned char group,int time,long waitMs)
{
    return setIntResp(id,group,CMD_SET_ADJ_TIME,time,1,waitMs);
}
int  LedCtrl::getAdjustTime(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_ADJ_TIME,1,waitMs);
}
int LedCtrl::getGroup(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_GROUP,1,waitMs);
}
int LedCtrl::getDefaultAdjValue(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_DEFAULT_BRIGHTNESS,1,waitMs);
}
int LedCtrl::setGroup(unsigned int id,unsigned char newGroup,unsigned int waitMs)
{
    return setIntResp(id,0,CMD_SET_GROUP,newGroup,1,waitMs);
}
int LedCtrl::setID(unsigned int id,unsigned int newId,unsigned int waitMs)
{
    return setIntResp(id,0,CMD_MODIFY_DEVID,newId,4,waitMs);
}
