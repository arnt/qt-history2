load(qttest_p4)
SOURCES  += tst_qsocks5socketengine.cpp


exists($$(QTDIR)/src/network/qnativesocketengine_p.h) {

include(../qnativesocketengine/qsocketengine.pri)

} #exists


MOC_DIR=tmp

QT = core network


DEFINES += QT_USE_USING_NAMESPACE

