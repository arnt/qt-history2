load(qttest_p4)

include(../src/src.pri)

DEFINES += QLOCALSERVER_DEBUG

SOURCES += tst_qlocalsocket.cpp
TARGET = tst_qlocalsocket

