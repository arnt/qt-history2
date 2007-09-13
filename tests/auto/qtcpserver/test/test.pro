load(qttest_p4)
SOURCES  += ../tst_qtcpserver.cpp

win32:LIBS += -lws2_32

TARGET = ../tst_qtcpserver

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qtcpserver
} else {
    TARGET = ../../release/tst_qtcpserver
  }
}

QT = core network

MOC_DIR=tmp

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}


DEFINES += QT_USE_USING_NAMESPACE

