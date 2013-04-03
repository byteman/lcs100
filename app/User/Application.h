#ifndef _Application_H
#define _Application_H


#define Data_Len 200
//#define Zigbee_CFG		 (1ul << 8)
//#define Zigbee_CFG_H()   LPC_GPIO1->DATA |=	 Zigbee_CFG
//#define Zigbee_CFG_L()	 LPC_GPIO1->DATA &= ~Zigbee_CFG
extern unsigned char  Version[];
extern uint32_t  AC_220Vvalue;
extern uint32_t  AC_220Avalue;
extern uint8_t Data_Buf[Data_Len];
extern void App_Command(void);//各个命令分解
extern void Tune_Light(void);
extern void Frame_OK(uint8_t command);
extern uint8_t App_CRC(void);//数据校验
extern uint32_t Measuring_220V(void);  //测电压
extern uint32_t Measuring_AC(void);    //测电流
extern void ZigbeeSetup(void);
extern void Inquiry_ID(void);
extern void SetupPwm(void);
extern void Inquiry_Version(void);
extern void SetupGroupNumber(void);
extern void InquiryGroupNumber(void);//查询组号
extern void InquiryLightValue(void);//查询灯光值
extern uint32_t Light_Measuring_220AV(void);//测量电压电流
extern void SetupCoefficient(void);
extern uint32_t Measuring_Pactive(void); //测有功功率
extern void SetZigbee(void);
extern void Modify_ID(void);
extern void deleteSetupGroupNumber(void);
extern void InquiryAcquiesceLightValue(void);
extern void Setup_lighttime(void);
extern void flicker(void);
extern void Setup_fieldlight(void);
extern void remove_fieldlight(void);
extern void in_field(void);
extern void Inquiry_fieldlightvalue(void);
extern void Inquiry_alldata(void);
extern void Inquiry_lighttime(void);
extern void upgrade(void);
extern void upgradedata(void);
extern void upgradecrc(void);
extern void reset(void);
extern void configuration(void);
extern void heartbeat(void);
extern void Delay1_MS(uint32_t ulTime);
extern void Delay1_US(uint32_t ulTime);
extern void Baud_Rate(void);
#endif


