// lcs100_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LedCtrl.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//LedCtrl::get().open("COM1",9600);

	return lcs100_SDKTest(argc,argv);
}

