#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T23:19:42
#
#-------------------------------------------------
include(qextserialport/src/qextserialport.pri)
QT       += core gui network

TARGET = AqLed
TEMPLATE = app


SOURCES += main.cpp\
        aqleds.cpp \
    comm/aqled.cpp \
    comm/protocol.cpp \
    comm/updater.cpp

HEADERS  += aqleds.h \
    comm/aqled.h \
    comm/protocol.h \
    comm/updater.h \
    comm/q6_binary.h

FORMS    += aqleds.ui

#ICON = aqleds.icns
