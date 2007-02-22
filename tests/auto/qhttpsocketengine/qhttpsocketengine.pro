load(qttest_p4)
SOURCES  += tst_qhttpsocketengine.cpp


exists($$(QTDIR)/src/network/qnativesocketengine_p.h) {

include(../qnativesocketengine/qsocketengine.pri)

} #exists


MOC_DIR=tmp

QT = core network

