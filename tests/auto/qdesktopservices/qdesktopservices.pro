CONFIG += qttest_p4

SOURCES		+= tst_qdesktopservices.cpp
TARGET		= tst_qdesktopservices

include(../src/qdesktopservices.pri)

DEFINES += QT_USE_USING_NAMESPACE

