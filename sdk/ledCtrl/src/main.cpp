#if 0
#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QTextCodec *codec = QTextCodec::codecForName("GB2312");

    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);

    MainWindow w;
    w.show();
    
    return a.exec();
}
#endif
#include <stdio.h>
#include "LedCtrl.h"
#include "Poco/Thread.h"
int main(int argc, char *argv[])
{
	DeviceList dl;
	dl.push_back(1);
	if(LedCtrl::get().open("COM5"))
	{
		
		printf("group=%d\r\n",LedCtrl::get().getGroup(1,1000));
		Poco::Thread::sleep(1000);
		printf("current=%d\r\n",LedCtrl::get().getCureent(1,1000));

		return 0;

		LedCtrl::get().upload("lcs100.bin",dl);

		
		while (1)
		{
			if(LedCtrl::get().hasUploadComplete())
			{
				printf("upload ok\r\n");
			}
			Poco::Thread::sleep(1000);
		}
	}
	
	
	return 0;
}
