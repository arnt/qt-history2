load(qttest_p4)

SOURCES += ../tst_qtcpsocket.cpp
win32:LIBS += -lws2_32
QT += network

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}

TARGET = ../tst_qtcpsocket

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qtcpsocket
} else {
    TARGET = ../../release/tst_qtcpsocket
  }
}
