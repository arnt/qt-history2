TEMPLATE	= lib
CONFIG		= qt staticlib release
HEADERS		= qftp.h \
		  qhttp.h \
		  qnetwork.h \
		  qserversocket.h \
		  qsocket.h \
		  qsocketdevice.h
SOURCES		= qftp.cpp \
		  qhttp.cpp \
		  qnetwork.cpp \
		  qserversocket.cpp \
		  qsocket.cpp \
		  qsocketdevice.cpp
TARGET		= qnetwork
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.1
