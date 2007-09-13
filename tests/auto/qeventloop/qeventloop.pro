load(qttest_p4)
SOURCES += tst_qeventloop.cpp
QT -= gui 
QT += network

win32:LIBS += -luser32

DEFINES += QT_USE_USING_NAMESPACE

