TEMPLATE	= lib
CONFIG		= qt staticlib release
HEADERS		= qdns.h \
		  qftp.h \
		  qhostaddress.h \
		  qnetwork.h \
		  qserversocket.h \
		  qsocket.h \
		  qsocketdevice.h
SOURCES		= qdns.cpp \
		  qftp.cpp \
		  qhostaddress.cpp \
		  qnetwork.cpp \
		  qserversocket.cpp \
		  qsocket.cpp \
		  qsocketdevice.cpp
TARGET		= qnetwork
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.8

unix:SOURCES   += qsocketdevice_unix.cpp
win32:SOURCES   += qsocketdevice_win.cpp
