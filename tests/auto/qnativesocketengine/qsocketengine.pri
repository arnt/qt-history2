QT += network

QNETWORK_SRC = $$(QTDIR)/src/network

INCLUDEPATH += $$QNETWORK_SRC

win32:LIBS += -lws2_32

unix:contains(QT_CONFIG, reduce_exports) {
    SOURCES += $$QNETWORK_SRC/qnativesocketengine_unix.cpp
    SOURCES += $$QNETWORK_SRC/qnativesocketengine.cpp
    SOURCES += $$QNETWORK_SRC/qabstractsocketengine.cpp
}
