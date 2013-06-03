#ifndef LEDCTRL_H
#define LEDCTRL_H
#include <iostream>
#include <vector>
#include "ledProto.h"
#ifdef _WIN32
#ifdef LEDCTRL_EXPORTS
    #define LEDCTRL_API __declspec(dllexport)
#else
    #define LEDCTRL_API __declspec(dllimport)
#endif
#else
    #define  LEDCTRL_API
#endif
/*！
zigbee 网络类型
*/
enum ZgbNetType{
    ZGB_NET_MESH=1, //网状网
    ZGB_NET_STAR=2, //星型网
    ZGB_NET_PEER=7 //对等网
};
/*！
zigbee 节点类型
*/
enum ZgbNodeType{
    ZGB_NODE_CORD=1, //中心节点
    ZGB_NODE_ROUTE=3, //中继路由
    ZGB_NODE_TERM=4 //终端节点
};

enum ZgbTxMode{
    ZGB_TX_BROADCAST=1, //广播
    ZGB_TX_MASTER_SLAVE=2, //主从
    ZGB_TX_P2P=3 //点对点
};

enum ZgbBaud{
    ZGB_BAUD_1200=1, //1200
    ZGB_BAUD_2400=2, //2400
    ZGB_BAUD_4800=3, //4800
    ZGB_BAUD_9600=4, //9600
    ZGB_BAUD_19200=5, //19200 [默认的通讯波特率]
    ZGB_BAUD_38400=6 //38400 [zigbee模块配置用的波特率]
};
enum ZgbParity{
    ZGB_PARITY_NONE=1, //无校验
	ZGB_PARITY_EVEN, //偶校验
    ZGB_PARITY_ODD //奇校验
};
enum ZgbDataBit{
    ZGB_DATABIT_8=1, //8个数据位
    ZGB_DATABIT_9=3 //9个数据位
};
enum ZgbDataMode{
    ZGB_DATA_ASCII=1, //数据采用ascii格式
    ZGB_DATA_HEX=2 //数据采用hex格式
};
/*!
数据源地址选项
*/
enum ZgbAddressMode{
    ZGB_ADDR_NONE=1, //不输出
    ZGB_ADDR_ASCII=2, //ASCII输出
    ZGB_ADDR_HEX=3 //16进制输出
};
typedef struct
{
    unsigned short  address; // 0-0xFFFF(其中0是cord的地址)
    unsigned char   netID; //0-0xFF
    ZgbNetType      netType;
    ZgbNodeType     nodeType;
    ZgbTxMode       txMode;
    ZgbBaud         baudRate;
    ZgbParity       parity;
    ZgbDataBit      dataBit;
    ZgbDataMode     dataMode;
    unsigned char   timeOut; //串口超时
    unsigned char   channal; //通信信道 0-0xFF
    unsigned char   kw; //发射功率
    ZgbAddressMode  addrMode;
} TZigbeeCfg;


#define MAX_SCENE_NUM 4
typedef struct
{
    int voltage; //电压
    int current; //电流
    int kw; //功率
    int brightness; //当前实时亮度值
    int defBright;  //默认的调光值
    int group; //设备所在的组
    int adjustTime; //调光时间
    int ver;    //单灯的版本号
    int devId; //设备编号
	int resetCnt; //复位次数
    TZigbeeCfg zigbeeCfg;
} StreetLight;

#define MAX_SENCE_NUM 10
typedef struct
{
    unsigned int  id;   //ID
    unsigned char group; //group
    unsigned int  brightness; //当前亮度
    unsigned int  def_brightness; //默认亮度
    unsigned int  adj_timeS; //调光时间
} LedParam;
/*!
事件类型
*/
enum LedEvent
{
    EV_UPLOAD_REQ,	//升级请求中...
    EV_UPLOAD_DATA, //升级数据传输中...
    EV_UPLOAD_VERIFY, //升级完毕，数据校验中...
    EV_UPLOAD_COMPLETE, //升级完毕，返回升级结果。
	EV_UPLOAD_TIMEOUT
};
/*！
错误代码
*/
enum LedError
{
    ERR_OK=0,
    ERR_UPLOAD_TOO_LARGE,       //升级文件太大
    ERR_UPLOAD_SESSION, //升级会话ID不一致
    ERR_UPLOAD_ERASE,   //升级擦除FLASH失败
    ERR_UPLOAD_WRITE_DATA, //升级时写入数据失败
    ERR_UPLOAD_CRC, //升级完成后校验升级文件失败
    ERR_UPLOAD_WRITE_CRC, //升级完成后写入文件CRC校验失败
    ERR_UPLOAD_PACKET_NUM //升级时收到错误的包序号
};
/*！
当前处于的工作模式
*/
enum LedMode
{
	MODE_BOOT=0,
	MODE_APP =1
};

struct TEventParam
{
    TEventParam(unsigned int _id,unsigned char _group,LedEvent _event,LedError _err=ERR_OK,unsigned int _arg=0)
    {
        id = _id;
        group = _group;
        event = _event;
        err = _err;
        arg = _arg;
    }
    unsigned int id;
    unsigned char group;
    LedEvent event;
    LedError err;
    unsigned int arg;
};


typedef void (*LedCallBackProc)(TEventParam* param,void* arg);



struct ILedEventNofityer
{
    virtual int notify(TEventParam* arg) = 0 ;
};
class LedMessage;
class  LEDCTRL_API LedCtrl
{
public:
    LedCtrl();
    static LedCtrl& get();
public:

    /*!
        \brief 设置回调函数，有任何异步事件发生的时候，就会回调通知接口
        \param[in] arg :附加参数
    */
    void addObserver(ILedEventNofityer* obs);
	bool close();
    /*!
    \brief 打开串口，初始化模块
    \return true: 成功 false:失败
    */
    bool open(const char* comPath,unsigned int bps);
    /*!
    \brief 升级单灯设备.
    \param[in] file :升级文件全路径.
    \param[in] devlist: 升级单灯设备ID数组.
	\param[in] devNum:  升级单灯设备ID数组的个数
    */
    bool upload(const char* file,unsigned int *devlist, int devNum);
	 /*!
    \brief 获取升级文件的版本号
   
    该函数需要在upload调用后，并且返回true后，才能正确获取到。
    */
	int  getUploadFileVersion(void);

    /*!
      判断升级操作是否已经结束
    */
    bool hasUploadComplete(void);

    /*!
      \brief 根据设备id，返回设备的当前所有实时参数
      \return NULL 无法获取该ID的参数
    */
    StreetLight*  getLightParam(unsigned int id);

    /*!
       \brief 修改单灯的ID
    */
    int setID(unsigned int id,unsigned int newId,unsigned int waitMs=1000);

    /*!
       \brief 通知设备[afterMs]ms后复位
       \param[in] afterMs 等待多少ms后复位
       \return 0 成功 <0 失败
    */
    int setDeviceReset(unsigned int id,unsigned char group,unsigned int afterMs=10,long waitMs = 1000);

    /*!
    \brief 调光指令
    \param[in] id 单灯的id
    \param[in] group 单灯的组号
    \param[in] value 调光值0-100
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
    \return 返回设置结果
    \retval <0 失败 ==0 成功
    */
    int  setBrigtness(unsigned int id,unsigned char group,unsigned char value,long waitMs=1000);

    /*!
    \brief 获取当前的亮度值
    \param[in] id 单灯的id
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
    \return 返回当前亮度
    \retval <0 失败 >=0 当前亮度
    */
    int  getBrigtness(unsigned int id,long waitMs=1000);


    /*!
    \brief 设置默认亮度
    \param[in] id 单灯的id
    \param[in] group 单灯的组号
    \param[in] value 调光值0-100
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
    \return 返回设置结果
    \retval <0 失败 ==0 成功
    */
    int  setDefaultBrigtness(unsigned int id,unsigned char group,unsigned char value,long waitMs=1000);

    /*!
    \brief 获取默认亮度值
    \param[in] id 单灯的id
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
    \return 返回默认亮度值
    \retval <0 失败 >=0 默认亮度
    */
    int  getDefaultBrigtness(unsigned int id,long waitMs=1000);


    /*!
    \brief 闪烁LED
    \param[in] id 单灯的id
    \param[in] group 单灯的组号
    \param[in] value 闪烁间隔时间(ms) 0-65535ms
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
    \return 返回
    \retval -1 超时失败 >=0 当前闪所时间
    */
    int  setShakeLed(unsigned int id,unsigned char group,unsigned short value,long waitMs=1000);

    /*!
    \brief 写入存储参数到eeprom中
    \param[in] id 单灯的id
    \param[in] group 单灯的组号
    \param[in] pPara 存储参数
    \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回,因为eeprom的操作较慢，所以时间稍微长点
    \return 返回结果
    \retval <0 失败 =0 成功
    */
    int  writeE2prom(unsigned int id,unsigned char group,LedParam* pPara,long waitMs=2000);
    /*!
        \brief 获取单灯实时电流
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回实时电流
        \retval <0 失败 >0 电流值
    */
    int  getCureent(unsigned int id,long waitMs=1000);
    /*!
        \brief 获取单灯实时电压
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回实时电压
        \retval <0 失败 >0 电压值
    */
    int  getVoltage(unsigned int id,long waitMs=1000);
    /*!
        \brief 获取单灯实时功率
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回实时功率
        \retval <0 失败 >0 功率值
    */
    int  getKw(unsigned int id,long waitMs=1000);
    /*!
        \brief 获取单灯zigbee配置
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回zigbee配置
        \retval NULL 失败 >0 zigbee配置
    */
    int  getZigbeeCfg(unsigned int id,TZigbeeCfg* cfg,long waitMs=1000);
    /*!
        \brief 设置单灯zigbee配置
        \param[in] id 单灯的id
        \param[in] pCfg zigbee配置
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回zigbee配置
        \retval NULL 失败 >0 zigbee配置
    */
    int  setZigbeeCfg(unsigned int id,TZigbeeCfg* pCfg,long waitMs=1000);


    /*!
        \brief 获取单灯固件版本号
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 返回固件版本号(1个字节) (例如0x31 表示 V3.1版本)
        \retval <=0 失败 >0 固件版本号
    */
    int  getVersion(unsigned int id,long waitMs=1000);
    /*!
        \brief 设置单灯调光时间
        \param[in] id 单灯的id
        \param[in] time 单灯调光时间(0-100)
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 调光时间
        \retval <0 失败 =0 成功
    */
    int  setAdjustTime(unsigned int id,unsigned char group,int time,long waitMs=1000);
    /*!
        \brief 获取单灯调光时间
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 调光时间
        \retval <0 失败 >=0 调光时间
    */
    int  getAdjustTime(unsigned int id,long waitMs=1000);

    int  getAllData(unsigned int id,unsigned char group,StreetLight* pLight,long waitMs=1000);

    /*!
        \brief 获取单灯默认调光值
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 默认调光值
        \retval <0 失败 >=0 默认调光值
    */
    int getDefaultAdjValue(unsigned int id,long waitMs=1000);

	int broadcastGetID(unsigned int waitMs=1000);
	/*!
		查询当前的工作模式
		0  ： 查询成功
		-1 ： 失败
	*/
	int getWorkMode(unsigned int id,unsigned int waitMs=1000);
public:
    /*!
       \brief 设置组号,修改当前单灯的组号为newGroup,其中组号0为无效组号，如果想把
       某个单灯从某个组删除，就将起组号设置为0.
    */
    int setGroup(unsigned int id,unsigned char newGroup,unsigned int waitMs=1000);

    /*!
        \brief 获取单灯所在的组号
        \param[in] id 单灯的id
        \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
        \return 组号 (组号为0，为无效组号)
        \retval <=0 失败 >0 组号
    */
    int getGroup(unsigned int id,long waitMs=1000);
    /*!
    内部使用
    */
    void notify(TEventParam* par);
    /*!
      获取升级文件总的文件包数
       */
    int  getUploadFilePacketNum(void);
    /*!
       \brief 获取单灯复位次数
       \param[in] id 单灯的id
       \param[in] waitMs 等待多少ms后如果都没有获取到数据就返回
       \return 返回复位次数
       \retval -1 超时失败 >=0 复位次数
    */
    int  getDeviceResetCount(unsigned int id,long waitMs=1000);
private:
    int  getIntResp(unsigned int id,LedCmdType type,int size,long waitMs=1000);
    int  setIntResp(unsigned int id,unsigned char group,LedCmdType type,int value,int size,long waitMs=1000);

    int  sendMessage(LedMessage* pMsg);
    bool waitRespMessage(LedMessage* pReqMsg,LedMessage* pRespMsg);
    bool checkPacketValid(LedMessage* pReqMsg,LedMessage* pRespMsg);
    unsigned char checkSum(unsigned char* buff, int size);
};
/*!
	SDK内部实现的一个测试SDK的各个接口的接口函数
*/
LEDCTRL_API int  lcs100_SDKTest(int argc, char *argv[]);
/*!
	运行时指定是否模拟运行
*/
LEDCTRL_API void lcs100_EnableSimulate(bool enable);
/*!
	判断当前是否模拟运行的
*/
LEDCTRL_API bool lcs100_IsSimulate(void);
#endif // LEDCTRL_H
