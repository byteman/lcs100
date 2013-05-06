#include "LedCtrl.h"
#include "LedUpload.h"
#include "ByProtocol.h"
#include "LedMsgQueue.h"
#include "serial/serial.h"
#include <Poco/SingletonHolder.h>
#include <Poco/ThreadPool.h>
#include <stdexcept>
#include <map>
#include <assert.h>

/*!
跨模块的接口中，参数和返回值都应该使用内置数据类型
接口不用使用stl，用基本类型
*/

#define ZIGBEE_CFG_SIZE 14
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
static TStreeLightList	simStreetList;
static StreetLight	 simStreetLight;
static bool isSimulate = false;
static bool bPortOpen = false;


void lcs100_EnableSimulate(bool enable)
{
    isSimulate = enable;
}
bool lcs100_IsSimulate(void)
{
    return isSimulate;
}
LedCtrl::LedCtrl()
{

}
int  getUploadFileVersion(void)
{
	return LedUpload::get().getFileVersion();
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
bool LedCtrl::close()
{
	printf("not support\n");
	return true;
}
bool LedCtrl::open(const char* comPath,unsigned int bps)
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

		bool ret = LedUpload::get ().setPort (pZigbeeCom);

		if(ret)
		{
			bPortOpen = true;
		}

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
	
#endif
	return false;
}
bool LedCtrl::upload(const char* file,unsigned int *devlist, int devNum)
{
    
	if (!bPortOpen) 
	{
		fprintf(stderr,"not open\r\n");
		return false;
	}

    if(!LedUpload::get ().loadUploadFile (file)) return false;

	std::vector<unsigned int> upLoadList;
	for(int i = 0 ; i < devNum ; i++)
	{
		upLoadList.push_back(devlist[i]);
	}

	return LedUpload::get ().startUploadFile (upLoadList);

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
    if(pZigbeeCom)
        return pZigbeeCom->write (context,totalLen);
    return 0;
}

int LedCtrl::setDeviceReset(unsigned int id,unsigned char group,unsigned int afterMs,long waitMs)
{
    return setIntResp(id,group,CMD_RESET,afterMs,2,waitMs);
}
bool LedCtrl::hasUploadComplete(void)
{
    return LedUpload::get().uploadHasComplete();
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
	if(pReqMsg->cmd == CMD_BROADCAST_DEVID) return (pReqMsg->cmd == pRespMsg->cmd);

    if(pReqMsg->id  != pRespMsg->id)  return false;
    if(pReqMsg->cmd != pRespMsg->cmd) return false;

    return true;
}

bool LedCtrl::waitRespMessage(LedMessage* pReqMsg,LedMessage* pRespMsg)
{
    unsigned char buf[128];

    if(!pReqMsg->needResp) return true;
    if(pZigbeeCom == NULL) return false;
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
void loadSimData(unsigned int id,unsigned group)
{
    static int adj_time = 20;
    static int brightness = 20;
    static int current = 20;
    static int defBright = 20;
    static int kw = 20;

    simStreetLight.adjustTime = adj_time++;
    simStreetLight.brightness = brightness++;
    simStreetLight.current	  = current++;
    simStreetLight.defBright  = defBright++;
    simStreetLight.devId	  = id;
	simStreetLight.group	  = group;
    simStreetLight.kw		  = kw++;
	simStreetLight.ver		  = 100;
    simStreetLight.resetCnt	  = 0;
}

StreetLight* getSimLight(unsigned int id,unsigned group)
{
	if(simStreetList.find(id) != simStreetList.end())
	{
		return &simStreetList[id];
	}
	loadSimData(id,group);
	simStreetList[id] = simStreetLight;
	return &simStreetList[id]; 
}
int getSimParam(unsigned int id,unsigned group,LedCmdType type)
{
	int value  = -1;
	switch(type)
	{
		case CMD_QUERY_BRIGHTNESS:
            value = getSimLight(id,group)->brightness;
			break;
		case CMD_GET_RESET_CNT:
            value = getSimLight(id,group)->resetCnt;
			break;
		case CMD_QUERY_DEFAULT_BRIGHTNESS:
            value = getSimLight(id,group)->defBright;
			break;
		case CMD_QUERY_CURRENT:
            value = getSimLight(id,group)->current;
			break;
        case CMD_QUERY_VOLTAGE:
            value = getSimLight(id,group)->voltage;
            break;
        case CMD_QUERY_VERSION:
            value = getSimLight(id,group)->ver;
            break;
        case CMD_QUERY_KW:
            value = getSimLight(id,group)->kw;
            break;
        case CMD_QUERY_ADJ_TIME:
            value = getSimLight(id,group)->adjustTime;
            break;
        case CMD_QUERY_GROUP:
            value = getSimLight(id,group)->group;
            break;
		default: break;
	}
	return value;
}
int setSimParam(unsigned int id,unsigned group,LedCmdType type,int value)
{

    switch(type)
    {
        case CMD_ADJUST_BRIGHTNESS:
            getSimLight(id,group)->brightness = value;
            break;
        case CMD_RESET:
            getSimLight(id,group)->resetCnt++;
            break;
        case CMD_SET_DEFAULT_BRIGHTNESS:
            getSimLight(id,group)->defBright = value;
            break;
        case CMD_SET_ADJ_TIME:
            getSimLight(id,group)->adjustTime = value;
            break;
        case CMD_SET_GROUP:
            getSimLight(id,group)->group = value;
            break;
        default:
            return -1;
            break;
    }
    return ERR_OK;
}
int  LedCtrl::getDeviceResetCount(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_GET_RESET_CNT,4,waitMs);
}

int  LedCtrl::getBrigtness(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_BRIGHTNESS,1,waitMs);
   
}
int  LedCtrl::setBrigtness(unsigned int id,unsigned char group,unsigned char value,long waitMs)
{
   return setIntResp(id,group,CMD_ADJUST_BRIGHTNESS,value,1,waitMs);
}
int  LedCtrl::getDefaultBrigtness(unsigned int id,long waitMs)
{
    return getIntResp(id,CMD_QUERY_DEFAULT_BRIGHTNESS,1,waitMs);
}
int  LedCtrl::setDefaultBrigtness(unsigned int id,unsigned char group,unsigned char value,long waitMs)
{
   return setIntResp(id,group,CMD_SET_DEFAULT_BRIGHTNESS,value,1,waitMs);
}
int  LedCtrl::setIntResp(unsigned int id,unsigned char group,LedCmdType type,int value,int size,long waitMs)
{
    if(isSimulate)
    {
        return setSimParam (id,group,type,value);
    }
    else
    {

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
unsigned int Buf2Int32(unsigned char* buff)
{
    return (buff[0]<<24) + (buff[1]<<16)+ (buff[2]<<16)+ (buff[3]<<0);
}
unsigned int Buf2Short16(unsigned char* buff)
{
    return (buff[0]<<16)+ (buff[1]<<0);
}
int  LedCtrl::getAllData(unsigned int id,unsigned char group,StreetLight* pLight,long waitMs)
{
    if(isSimulate)
    {
        StreetLight* _pTemp = getSimLight(id,group);

        if(_pTemp != NULL)
        {
            memcpy(pLight,_pTemp,sizeof(StreetLight));

            return ERR_OK;
        }
        return  -1;
    }
    else
    {

        LedMessage respMsg;
        LedMessage reqMsg(id,0,CMD_QUERY_ALL,true,25,waitMs);

        sendMessage (&reqMsg);

        if( waitRespMessage (&reqMsg,&respMsg))
        {
            if(respMsg.respCode == ERR_OK)
            {
                int len = 0;
                unsigned char* pData = respMsg.getBuffVal (len);
                if(len == 25)
                {
                    pLight->voltage = Buf2Int32(pData);
                    pLight->current = Buf2Int32(pData+4);
                    pLight->kw = Buf2Int32(pData+8);
                    pLight->brightness = pData[12];
                    pLight->defBright = pData[13];
                    pLight->adjustTime = pData[14];
                    pLight->group = pData[15];
                    pLight->devId = Buf2Int32(pData+16);
					pLight->ver  = pData[20];
					pLight->resetCnt = Buf2Int32(pData+21);
                    return ERR_OK;
                }

            }
        }
    }
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
//FIXME
int  LedCtrl::setZigbeeCfg(unsigned int id,TZigbeeCfg* pCfg,long waitMs)
{
	if(isSimulate)
	{
        StreetLight* _pTemp = getSimLight(id,0);

        if(_pTemp != NULL)
        {
            //memcpy(pLight->zigbeeCfg,_pTemp,sizeof(StreetLight));
            _pTemp->zigbeeCfg = *pCfg;
            return ERR_OK;
        }
        return  -1;
	}
	else
	{
		LedMessage respMsg;
		LedMessage reqMsg(id,0,CMD_SET_ZIGBEE_CFG,true,0,waitMs);

		sendMessage (&reqMsg);

		if( waitRespMessage (&reqMsg,&respMsg))
		{
		    if(respMsg.respCode == ERR_OK)
		    {
                int len = 0;
                unsigned char* pData = respMsg.getBuffVal (len);
                if(len > 0)
                {


                }
		        return 0;
		    }
		    return -respMsg.respCode;

		}
	}
    return -1;
}

int  LedCtrl::getZigbeeCfg(unsigned int id,TZigbeeCfg* cfg,long waitMs)
{
    if(isSimulate)
    {
        StreetLight* _pTemp = getSimLight(id,0);

        if(_pTemp != NULL)
        {
            //memcpy(pLight->zigbeeCfg,_pTemp,sizeof(StreetLight));
            *cfg = _pTemp->zigbeeCfg;
            return ERR_OK;
        }
        return  -1;
    }
    else
    {
        LedMessage respMsg;
        LedMessage reqMsg(id,0,CMD_QUERY_ZIGBEE_CFG,true,sizeof(TZigbeeCfg),waitMs);

        sendMessage (&reqMsg);

        if( waitRespMessage (&reqMsg,&respMsg))
        {
            if(respMsg.respCode == ERR_OK)
            {
                int len = 0;
                unsigned char* pData = respMsg.getBuffVal (len);

                if(len != ZIGBEE_CFG_SIZE) return NULL;

                cfg->address    = Buf2Short16 (pData);
                cfg->netID      = pData[2];
                cfg->netType    = (ZgbNetType)pData[3];
                cfg->nodeType   = (ZgbNodeType)pData[4];
                cfg->txMode     = (ZgbTxMode)pData[5];
                cfg->baudRate   = (ZgbBaud)pData[6];
                cfg->parity     = (ZgbParity)pData[7];
                cfg->dataBit    = (ZgbDataBit)pData[8];
                cfg->dataMode   = (ZgbDataMode)pData[9];
                cfg->timeOut    = pData[10];
                cfg->channal    = pData[11];
                cfg->kw         = pData[12];
                cfg->addrMode   = (ZgbAddressMode)pData[13];
                return ERR_OK;
            }

        }
    }

    return -1;
}
int  LedCtrl::getIntResp(unsigned int id,LedCmdType type,int size,long waitMs)
{
    if(isSimulate)
    {
        return getSimParam(id,0,type);
    }
    else
    {

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
int LedCtrl::broadcastGetID(unsigned int waitMs)
{
	return getIntResp(0,CMD_BROADCAST_DEVID,4,waitMs);
}