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
           qserversocket.h \
           qsocket.h \
           qsocketdevice.h \
           qsocketdevice_p.h \
           qdns.h \
           qdns_p.h \
           qurlinfo.h

SOURCES	= qftp.cpp \
          qhttp.cpp \
          qhostaddress.cpp \
          qserversocket.cpp \
          qsocket.cpp \
          qsocketdevice.cpp \
          qdns.cpp \
          qurlinfo.cpp

unix:SOURCES += qdns_unix.cpp qsocketdevice_unix.cpp
win32:SOURCES += qdns_win.cpp qsocketdevice_win.cpp

mac:INCLUDEPATH += ../3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)

QMAKE_LIBS += $$QMAKE_LIBS_NETWORK
