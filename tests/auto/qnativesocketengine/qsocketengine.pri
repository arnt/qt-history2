QT += network

QNETWORK_SRC = $$(QTDIR)/src/network

INCLUDEPATH += $$QNETWORK_SRC

SOURCES += $$QNETWORK_SRC/qabstractsocketengine.cpp \
           $$QNETWORK_SRC/qnativesocketengine.cpp \
           $$QNETWORK_SRC/qsocks5socketengine.cpp


HEADERS += $$QNETWORK_SRC/qabstractsocketengine_p.h \
           $$QNETWORK_SRC/qnativesocketengine_p.h \
           $$QNETWORK_SRC/qsocks5socketengine_p.h


win32 {
    SOURCES += $$QNETWORK_SRC/qnativesocketengine_win.cpp
    LIBS += -lws2_32
}

unix:contains(QT_CONFIG, reduce_exports) {
    SOURCES += $$QNETWORK_SRC/qnativesocketengine_unix.cpp
}