load(qttest_p4)
SOURCES += ../tst_qdbusmarshall.cpp
TARGET = ../tst_qdbusmarshall

QT = core
CONFIG += qdbus

DEFINES += QT_USE_USING_NAMESPACE

