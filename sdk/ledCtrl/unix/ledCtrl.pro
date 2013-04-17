#-------------------------------------------------
#
# Project created by QtCreator 2013-04-03T14:49:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ledCtrl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    LedUpload.cpp \
    crc.c \
    ByProtocol.c \
    LedCtrl.cpp \
    src/serial.cpp \
    src/impl/unix.cpp \
    LedMsgQueue.cpp

HEADERS  += mainwindow.h \
    LedUpload.h \
    crc.h \
    ByProtocol.h \
    LedCtrl.h \
    include/serial/serial.h \
    ledProto.h \
    LedMsgQueue.h
INCLUDEPATH +=./include/
FORMS    += mainwindow.ui

LIBS+=-lPocoFoundation -lserial
