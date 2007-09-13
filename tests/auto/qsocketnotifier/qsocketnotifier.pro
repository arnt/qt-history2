load(qttest_p4)
SOURCES += tst_qsocketnotifier.cpp
QT = core network

exists($$(QTDIR)/src/network/qnativesocketengine_p.h) {

include(../qnativesocketengine/qsocketengine.pri)

} #exists


DEFINES += QT_USE_USING_NAMESPACE

