load(qttest_p4)
SOURCES  += tst_qhostaddress.cpp


QT = core network

win32:LIBS += -lws2_32
