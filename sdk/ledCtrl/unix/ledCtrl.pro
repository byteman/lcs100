#TEMPLATE = app
TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../src/main.cpp \
    ../src/LedUpload.cpp \
    ../src/LedMsgQueue.cpp \
    ../src/LedCtrl.cpp \
    ../src/crc.c \
    ../src/ByProtocol.c \
    ../serial/src/serial.cpp \
    ../serial/src/impl/unix.cpp

HEADERS += \
    ../src/LedUpload.h \
    ../src/LedMsgQueue.h \
    ../src/crc.h \
    ../src/ByProtocol.h \
    ../include/ledProto.h \
    ../include/LedCtrl.h
INCLUDEPATH += ../include ../serial/include

linux-g++ {
    message(g++ = linux-g++)
}
linux-armv7-g++ {
    message(g++ = linux-arm-g++ compile)
    INCLUDEPATH += /media/linuxdata/home/byteman/library/Poco-ARM/include
    LIBS += -L/media/linuxdata/home/byteman/library/Poco-ARM/lib
}
LIBS +=-lPocoFoundation

