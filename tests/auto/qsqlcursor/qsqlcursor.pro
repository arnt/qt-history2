load(qttest_p4)
SOURCES  += tst_qsqlcursor.cpp

QT += sql qt3support

win32:LIBS += -lws2_32


DEFINES += QT_USE_USING_NAMESPACE

