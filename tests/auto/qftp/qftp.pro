load(qttest_p4)
SOURCES  += tst_qftp.cpp


QT = core network

exists($$(QTDIR)/src/network/qnetworkproxy.h) {
    DEFINES += TEST_QNETWORK_PROXY
}


DEFINES += QT_USE_USING_NAMESPACE

