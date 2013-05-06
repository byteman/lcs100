#ifndef LED_PROTO_INCLUDED
#define LED_PROTO_INCLUDED

#define PROTO_HEAD 0xA3

enum LedError{
    ERR_OK=0,
    ERR_TOO_LARGE,
    ERR_SESSION,
    ERR_ERASE,
    ERR_WRITE_DATA,
    ERR_CRC,
    ERR_WRITE_CRC,
    ERR_PACKET_NUM
};

enum LedMode
{
	MODE_BOOT=0,
	MODE_APP =1
};

enum   LedCmdType
{
    CMD_NONE=0,     //无效升级命令
    CMD_UPLOAD_REQ, //升级请求
    CMD_UPLOAD_DATA,    //升级数据包
    CMD_UPLOAD_VERIFY,  //升级数据校验
    CMD_BOOTMODE,   //进入升级模式
    CMD_RESET,  //设备复位
    CMD_ADJUST_BRIGHTNESS, //调光
    CMD_TWINKLE, //闪烁
    CMD_HEART, //心跳包
    CMD_SET_ADJ_TIME, //设置调光时间
    CMD_SET_DEFAULT_BRIGHTNESS, //设置默认调光值
    CMD_SET_GROUP, //设置组号
    CMD_REMOVE_GRUOP, //将该设备从该组删除
    CMD_SET_SCENE_BRIGHTNESS, //设置某个场景的亮度
    CMD_REMOVE_SCENE, //删除某个场景
    CMD_SET_BAUD,//设置通讯波特率
    CMD_ENTER_SCENE, //进入某个场景
    CMD_WRITE_EEPROM, //烧写EEPROM
    CMD_MODIFY_DEVID, //修改终端ID
    CMD_QUERY_VOLTAGE,  //查询电压
    CMD_QUERY_CURRENT, //查询电流
    CMD_QUERY_KW,   //查询有功功率
    CMD_QUERY_BRIGHTNESS, //查询直接调光值
    CMD_QUERY_DEFAULT_BRIGHTNESS,//查询默认调光值
    CMD_QUERY_ADJ_TIME, //查询调光时间
    CMD_QUERY_GROUP,    //查询组号
    CMD_QUERY_SCENE_BRIGHTNESS, //查询某个场景的亮度
    CMD_QUERY_DEVICE_STATUS, //查询设备工作状态有无故障之类
    CMD_QUERY_VERSION,  //查询单灯的版本号
    CMD_QUERY_ALL, //查询所有单灯的工作参数，节约通讯量。
    CMD_BROADCAST_DEVID, //广播查询所有设备ID
    CMD_SET_ZIGBEE_CFG,  //对单灯的zigbee设备进行配置
    CMD_GET_RESET_CNT, //获取单灯复位次数
    CMD_QUERY_ZIGBEE_CFG,
	CMD_QUERY_MODE, //查询当前模式 [0: boot模式和1: app模式]
};


#endif
