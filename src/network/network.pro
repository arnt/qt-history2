TARGET   = QtNetwork
QPRO_PWD = $$PWD
DEFINES += QT_BUILD_NETWORK_LIB
include(../qbase.pri)

QT = core

# Qt network module

PRECOMPILED_HEADER = ../core/global/qt_pch.h
HEADERS += qftp.h \
           qhttp.h \
           qhostaddress.h \
           qsocketlayer_p.h \
           qabstractsocket.h \
           qabstractsocket_p.h \
           qtcpsocket.h \
           qudpsocket.h \
           qtcpserver.h \
           qdns.h \
           qdns_p.h \
           qurlinfo.h

SOURCES	= qftp.cpp \
          qhttp.cpp \
          qhostaddress.cpp \
          qsocketlayer.cpp \
          qabstractsocket.cpp \
          qtcpsocket.cpp \
          qudpsocket.cpp \
          qtcpserver.cpp \
          qdns.cpp \
          qurlinfo.cpp

unix:SOURCES += qdns_unix.cpp qsocketlayer_unix.cpp
win32:SOURCES += qdns_win.cpp qsocketlayer_win.cpp

mac:INCLUDEPATH += ../3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)

QMAKE_LIBS += $$QMAKE_LIBS_NETWORK
