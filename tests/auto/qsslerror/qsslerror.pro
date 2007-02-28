load(qttest_p4)

SOURCES += tst_qsslerror.cpp
win32:LIBS += -lws2_32
QT += network

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}

TARGET = tst_qsslerror

win32 {
  CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
  }
}
