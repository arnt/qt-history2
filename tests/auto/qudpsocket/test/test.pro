load(qttest_p4)
SOURCES  += ../tst_qudpsocket.cpp
QT = core network

MOC_DIR=tmp

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}

TARGET = ../tst_qudpsocket
