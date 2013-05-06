TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += ../include

linux-g++ {
    message(g++ = linux-g++)
}
linux-armv7-g++ {
    message(g++ = linux-arm-g++ compile)
    LIBS += -L/media/linuxdata/home/byteman/library/Poco-ARM/lib
}
LIBS+= -L.
LIBS+= -lledCtrl -lPocoFoundation
DESTDIR = ./
