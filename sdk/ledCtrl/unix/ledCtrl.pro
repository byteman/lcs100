TEMPLATE = app
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
INCLUDEPATH = ../include ../serial/include

LIBS +=-lPocoFoundation
