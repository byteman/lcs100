#include <stdio.h>
#include "LedCtrl.h"
#include "Poco/Thread.h"

int gTermId = 1;
unsigned char gGroup = 0;
bool gQuit = false;

typedef bool (*TestProc)(void);
typedef struct
{
    const char* itemText;
    TestProc	proc;
} TestItem;




bool testReadAll()
{
    TZigbeeCfg* cfg;
    printf("group=%d\r\n",LedCtrl::get().getGroup(gTermId));
    printf("current=%d\r\n",LedCtrl::get().getCureent(gTermId));
    printf("adj time=%d\r\n",LedCtrl::get().getAdjustTime(gTermId));
    printf("default adj time=%d\r\n",LedCtrl::get().getDefaultAdjValue(gTermId));

    printf("reset counter=%d\r\n",LedCtrl::get().getDeviceResetCount(gTermId));
    printf("KW=%d\r\n",LedCtrl::get().getKw(gTermId));
    printf("version=%d\r\n",LedCtrl::get().getVersion(gTermId));
    printf("Voltage=%d\r\n",LedCtrl::get().getVoltage(gTermId));

    cfg = LedCtrl::get().getZigbeeCfg(gTermId);
    if (cfg)
    {
        printf("zigbee addr=%d\r\n",cfg->address);
    }

    return true;
}
class UploadTest:public ILedEventNofityer
{
public:
    int notify(TEventParam* arg)
    {
        if(arg)
            printf("event=%d\n",arg->event);
        return 0;
    }
};
UploadTest ut;
bool testUpload()
{
    int  timeout = 60;
    DeviceList dl;
    dl.push_back(1);


    LedCtrl::get().addObserver(&ut);
    LedCtrl::get().upload("lcs100.bin",dl);


    while (!LedCtrl::get().hasUploadComplete() && timeout--)
    {
        Poco::Thread::sleep(1000);
    }

    return timeout?true:false;
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

static TestItem testList[] =
{
    {"test Upload file",testUpload},
    {"read all param",testReadAll},
    {"set group",setGroup},
    {"set adj time",setAdjtime},
    {"set brightness",setBrightness},
    {"reset device",setReset},
    {"quit app",quitApp},
};

void displayHelp(void)
{
    size_t i =0;
    for (; i < sizeof(testList)/sizeof(TestItem); i++)
    {
        printf("[%d].%s\r\n",i,testList[i].itemText);
    }

    int ch = 0;
    scanf("%d",&ch);

    if(ch >=0 && ch <= i)
    {
        if(testList[ch].proc)
            testList[ch].proc();
    }

}

int main(int argc, char *argv[])
{
    //char* comPath = "COM5";
    char* comPath = "/dev/ttyUSB0";
    if(argc == 2)
    {
        //fprintf(stderr,"usage: lcs100.exe COM[n]\r\n");
        //return -1;
        comPath =  argv[1];
    }


    if(!LedCtrl::get().open(comPath))
    {
        fprintf(stderr,"Open ComPort Failed\r\n");
        return 0;

    }
    while (!gQuit)
    {
        displayHelp();
    }


    return 0;
}
