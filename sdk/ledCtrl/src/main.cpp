#include <stdio.h>
#include "LedCtrl.h"
#include "Poco/Thread.h"
#include <assert.h>
int gTermId = 1;
unsigned char gGroup = 5;
bool gQuit = false;

typedef bool (*TestProc)(void);
typedef struct
{
    const char* itemText;
    TestProc	proc;
} TestItem;


bool assertTest()
{
    int testCnt = 0;
	StreetLight slt;
    while(1)
    {
#if 0
		assert( LedCtrl::get().getDeviceResetCount(gTermId) >= 0);
		assert( LedCtrl::get().getGroup(gTermId) >= 0);
		assert( LedCtrl::get().getCureent(gTermId) >= 0);
		assert( LedCtrl::get().getAdjustTime(gTermId) >= 0);
		assert( LedCtrl::get().getDefaultAdjValue(gTermId) >= 0);
		assert( LedCtrl::get().getBrigtness(gTermId) >= 0);
		assert( LedCtrl::get().getKw(gTermId) >= 0);
		assert( LedCtrl::get().getVersion(gTermId) >= 0);
#else
		printf("all data=%d\n",LedCtrl::get().getAllData(gTermId,0,&slt));
		//assert(LedCtrl::get().getAllData(gTermId,0,&slt) = -1);
#endif
		printf("test ok =%d\n",testCnt++);
        Poco::Thread::sleep (1000);
    }


}

bool testReadRealTimeAll()
{
    TZigbeeCfg* cfg;
    printf("reset counter=%d\r\n",LedCtrl::get().getDeviceResetCount(gTermId));
	printf("brightness=%d\r\n",LedCtrl::get().getBrigtness (gTermId));

    printf("Voltage=%d\r\n",LedCtrl::get().getVoltage(gTermId));
    printf("group=%d\r\n",LedCtrl::get().getGroup(gTermId));
    printf("current=%d\r\n",LedCtrl::get().getCureent(gTermId));
    printf("adj time=%d\r\n",LedCtrl::get().getAdjustTime(gTermId));
    printf("default adj time=%d\r\n",LedCtrl::get().getDefaultAdjValue(gTermId));
    

    printf("KW=%d\r\n",LedCtrl::get().getKw(gTermId));
    printf("version=%d\r\n",LedCtrl::get().getVersion(gTermId));

#if 0
    cfg = LedCtrl::get().getZigbeeCfg(gTermId);
    if (cfg)
    {
        printf("zigbee addr=%d\r\n",cfg->address);
    }
#endif
    return true;
}
bool testReadAll()
{
    TZigbeeCfg* cfg;

	StreetLight Light;
	if( ERR_OK != LedCtrl::get().getAllData(gTermId,gGroup,&Light))
	{
		printf("get All data failed\r\n");
		return false;
	}
	printf("reset counter=%d\r\n",Light.resetCnt);
	printf("brightness=%d\r\n",Light.brightness);

	printf("Voltage=%d\r\n",Light.voltage);
	printf("group=%d\r\n",Light.group);
	printf("current=%d\r\n",Light.current);
	printf("adj time=%d\r\n",Light.adjustTime);
	printf("default brightness=%d\r\n",Light.defBright);
    

	printf("KW=%d\r\n",Light.kw);
	printf("version=%d\r\n",Light.ver);


    //printf("zigbee addr=%d\r\n",cfg->address);
    

    return true;
}

class UploadTest:public ILedEventNofityer
{
public:
    int notify(TEventParam* arg)
    {
        if(arg)
        {
            printf("event=%d\n",arg->event);
            switch(arg->event)
            {
                case EV_UPLOAD_REQ:
                    printf("upload request\n");
                    break;
                case EV_UPLOAD_DATA:
                    printf("upload data [%d]-[%d]\n",(arg->arg)>>16,(arg->arg)&0xFFFF);
                    break;
                case EV_UPLOAD_VERIFY:
                    printf("upload verify\n");
                    break;
                case EV_UPLOAD_COMPLETE:
                    printf("upload complete ver=%d\n",arg->arg);
                    break;
                default:
                    break;
            }
        }

        return 0;
    }
};
UploadTest ut;
bool testUpload()
{
    int  timeout = 60;
    std::vector<int> dl;

	printf("input upload id\n");
	int tmp;

	scanf("%d",&tmp);

    dl.push_back(tmp);
	//dl.push_back(2);
	//dl.push_back(3);
	
    LedCtrl::get().addObserver(&ut);
	LedCtrl::get().upload("lcs100.bin",(unsigned int*)dl.data(),dl.size());
    //LedCtrl::get().upload("D:\\proj\\2013\\led\\lcs100\\sdk\\ledCtrl\\windows\\outputD\\lcs100.bin",(unsigned int*)dl.data(),dl.size());
#if 0
    printf ("upload return\n");
    while (!LedCtrl::get().hasUploadComplete())
    {
        printf("uploading.....\n");
        Poco::Thread::sleep(1000);
        timeout--;
        if(timeout == 0) break;
    }
    printf("timeout=%d\n",timeout);
    printf("upload end.....%s\n",(timeout<=0)?"timeout":"successful");

    return timeout?true:false;
#endif
	return true;
}

bool quitApp()
{
    gQuit = true;
    return true;
}

bool setGroup()
{
    int group = 0;
    printf("input group number\r\n");
    scanf("%d",&group);
    if (LedCtrl::get().setGroup(gTermId,group) == ERR_OK)
    {
        printf("set ok\r\n");
    }
    return true;
}
bool setAdjtime()
{
    int value = 0;
    printf("input adj time\r\n");
    scanf("%d",&value);
    if (LedCtrl::get().setAdjustTime(gTermId,gGroup,value) == ERR_OK)
    {
        printf("set ok\r\n");
    }
    return true;
}
bool setDefaultBrightness()
{
    int value = 0;
    printf("input default brightness\r\n");
    scanf("%d",&value);

    if (LedCtrl::get().setDefaultBrigtness (gTermId,gGroup,value) == ERR_OK)
    {
        printf("set ok\r\n");
    }
    return true;
}
bool setBrightness()
{
    int value = 0;
    printf("input cur brightness\r\n");
    scanf("%d",&value);
    if (LedCtrl::get().setBrigtness(gTermId,gGroup,value) == ERR_OK)
    {
        printf("set ok\r\n");
    }
    return true;
}
bool setReset()
{
    int value = 0;
    printf("input reset time\r\n");
    scanf("%d",&value);
    if (LedCtrl::get().setDeviceReset(gTermId,gGroup,value) == ERR_OK)
    {
        printf("set ok\r\n");
    }
    return true;
}
bool changeGroupAndID()
{
    int tmp = 0;
    printf("please input new ID\n");
    scanf("%d",&tmp);
    if(tmp > 0)
    {
        gTermId =  tmp ;
        return true;
    }
    return false;
}
bool updateGroupAndID()
{
	int tmp = 0;
	printf("please input new ID\n");
	scanf("%d",&tmp);
	if(tmp > 0)
	{
		LedCtrl::get().setID(gTermId,tmp);
		return true;
	}
	return false;
}
bool queryMode(void)
{
#if 1
	int ret = LedCtrl::get().getWorkMode(gTermId);
	if (ret == -1)
	{
		printf("read timeout\r\n");
		return false;
	}

	printf("Mode=%s\r\n",(ret==MODE_BOOT)?"BOOT":"APP");
#endif
	return true;
}
bool broadCastID(void)
{
	printf("broad cast id=%d\n",LedCtrl::get().broadcastGetID());

	return true;
}
bool enableSimu(void)
{
    bool isEn = lcs100_IsSimulate();

    //printf("press anykey to change\n");
    //int tmp = 0;
    //scanf("%d",&tmp);
    isEn = !isEn;
    lcs100_EnableSimulate (isEn);

    printf("current [%s] simulate\n",isEn?"en":"disEn");

    return true;
}
static TestItem testList[] =
{
    {"test Upload file",testUpload},
    {"read all realtime param",testReadRealTimeAll},
	{"read all param",testReadAll},
    {"set group",setGroup},
    {"set adj time",setAdjtime},
    {"set brightness",setBrightness},
    {"set default brightness",setDefaultBrightness},
    {"reset device",setReset},
    {"assert all",assertTest},
    {"change ID",changeGroupAndID},
	{"update id",updateGroupAndID},
    {"enable/disable simulate",enableSimu},
	{"broadcast query id",broadCastID},
	{"query mode",queryMode},
    {"quit app",quitApp},
};

void displayHelp(void)
{
    size_t i =0;
    printf("current ID=%d,group=%d\n",gTermId,gGroup);
    for (; i < sizeof(testList)/sizeof(TestItem); i++)
    {
        printf("[%d].%s\r\n",i,testList[i].itemText);
    }

    int ch = 0;
    int ret = scanf("%d",&ch);
    //assert(ret == 0);
    if(ch >=0 && ch <= i)
    {
        if(testList[ch].proc)
            testList[ch].proc();
    }

}

int lcs100_SDKTest(int argc, char *argv[])
{
#if defined(_WIN32)
    const char* comPath = "COM2";
#elif defined(__GNUC__)
	const char* comPath = "/dev/ttyUSB0";
#endif
	if(argc == 2)
	{
		//fprintf(stderr,"usage: lcs100.exe COM[n]\r\n");
		//return -1;
		comPath =  argv[1];
	}

	//lcs100_EnableSimulate(true);
	if(!LedCtrl::get().open(comPath,19200))
	{

        fprintf(stderr,"Open ComPort[%s] Failed\r\n",comPath);
		fprintf(stderr,"usage: lcs100.exe COM[n]\r\n");
        return 0;
	}
	while (!gQuit)
	{
		displayHelp();
	}


	return 0;
}
#if 1
int main(int argc, char *argv[])
{
    return lcs100_SDKTest(argc,argv);
}
#endif
