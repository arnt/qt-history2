# Qt network module

network {
	win32:NETWORK_H	= ../include
	unix:NETWORK_H	= network
	unix:DEPENDPATH += :$$NETWORK_H
	HEADERS += $$NETWORK_H/qdns.h \
		    $$NETWORK_H/qftp.h \
		    $$NETWORK_H/qhttp.h \
		    $$NETWORK_H/qhostaddress.h \
		    $$NETWORK_H/qnetwork.h \
		    $$NETWORK_H/qserversocket.h \
		    $$NETWORK_H/qsocket.h \
		    $$NETWORK_H/qsocketdevice.h
	NETWORK_SOURCES	= network/qdns.cpp \
		    network/qftp.cpp \
		    network/qhttp.cpp \
		    network/qhostaddress.cpp \
		    network/qnetwork.cpp \
		    network/qserversocket.cpp \
		    network/qsocket.cpp \
		    network/qsocketdevice.cpp
	unix:NETWORK_SOURCES += network/qsocketdevice_unix.cpp
	win32:NETWORK_SOURCES += network/qsocketdevice_win.cpp
	SOURCES    += $$NETWORK_SOURCES
}