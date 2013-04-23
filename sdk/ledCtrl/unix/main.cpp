#include <iostream>
#include "LedCtrl.h"
using namespace std;

int main(int argc, char *argv[])
{
    lcs100_EnableSimulate (true);
    return lcs100_SDKTest (argc,argv);
}

