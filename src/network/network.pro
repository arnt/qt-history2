REQUIRES = !qt_one_lib
TARGET = qnetwork

DEFINES += QT_BUILD_NETWORK_LIB

include(../qbase.pri)
QCONFIG = core 

# Don't link against gui
win32:QCONFIG+=gui

# Qt network module

network {
	PRECOMPILED_HEADER = ../core/global/qt_pch.h
	HEADERS += qftp.h \
		    qhttp.h \
		    qhostaddress.h \
		    qserversocket.h \
		    qsocket.h \
		    qsocketdevice.h \
		    qresolver.h \
		    qresolver_p.h \
		    qurlinfo.h

	SOURCES	= qftp.cpp \
		    qhttp.cpp \
		    qhostaddress.cpp \
		    qserversocket.cpp \
		    qsocket.cpp \
		    qsocketdevice.cpp \
		    qresolver.cpp \
		    qurlinfo.cpp

	unix:SOURCES += qsocketdevice_unix.cpp qresolver_unix.cpp
	win32:SOURCES += qsocketdevice_win.cpp qresolver_win.cpp

	mac:INCLUDEPATH += ../3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)
}
