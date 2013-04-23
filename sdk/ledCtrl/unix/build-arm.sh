qmake -spec qws/linux-armv7-g++ ledCtrl.pro
make clean
make

qmake -spec qws/linux-armv7-g++ test.pro
make clean
make

