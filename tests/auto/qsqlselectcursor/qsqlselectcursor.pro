load(qttest_p4)
SOURCES  += tst_qsqlselectcursor.cpp

QT += sql qt3support

win32:LIBS += -lws2_32

