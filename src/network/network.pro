TARGET   = QtNetwork
QPRO_PWD = $$PWD
DEFINES += QT_BUILD_NETWORK_LIB
QT = core
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x64000000

include(../qbase.pri)

# Qt network module

PRECOMPILED_HEADER = ../corelib/global/qt_pch.h
HEADERS += qftp.h \
           qhttp.h \
           qhostaddress.h \
           qsocketlayer_p.h \
           qabstractsocket.h \
           qabstractsocket_p.h \
           qtcpsocket.h \
           qudpsocket.h \
           qtcpserver.h \
           qhostinfo.h \
           qhostinfo_p.h \
           qurlinfo.h

SOURCES	= qftp.cpp \
          qhttp.cpp \
          qhostaddress.cpp \
          qsocketlayer.cpp \
          qabstractsocket.cpp \
          qtcpsocket.cpp \
          qudpsocket.cpp \
          qtcpserver.cpp \
          qhostinfo.cpp \
          qurlinfo.cpp

unix:SOURCES += qhostinfo_unix.cpp qsocketlayer_unix.cpp
win32:SOURCES += qhostinfo_win.cpp qsocketlayer_win.cpp


mac:INCLUDEPATH += ../3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)

QMAKE_LIBS += $$QMAKE_LIBS_NETWORK
