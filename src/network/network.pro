REQUIRES = !qt_one_lib
TARGET = qnetwork

DEFINES += QT_BUILD_NETWORK_LIB

include(../qbase.pri)
QCONFIG = core compat

# Qt network module

network {
	HEADERS += qdns.h \
		    qftp.h \
		    qhttp.h \
		    qhostaddress.h \
		    qserversocket.h \
		    qsocket.h \
		    qsocketdevice.h \
		    qurlinfo.h

	SOURCES	= qdns.cpp \
		    qftp.cpp \
		    qhttp.cpp \
		    qhostaddress.cpp \
		    qserversocket.cpp \
		    qsocket.cpp \
		    qsocketdevice.cpp \
		    qurlinfo.cpp

	unix:SOURCES += qsocketdevice_unix.cpp
	win32:SOURCES += qsocketdevice_win.cpp

	mac:INCLUDEPATH += 3rdparty/dlcompat #qdns.cpp uses it (on Jaguar)
}
