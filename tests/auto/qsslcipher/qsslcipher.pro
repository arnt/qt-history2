load(qttest_p4)

SOURCES += tst_qsslcipher.cpp
win32:LIBS += -lws2_32
QT += network

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}

TARGET = tst_qsslcipher

win32 {
  CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
  }
}
