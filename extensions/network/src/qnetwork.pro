TEMPLATE	= lib
CONFIG		= qt staticlib release debug
unix:TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
HEADERS		= qdns.h \
		  qftp.h \
		  qhostaddress.h \
		  qhttp.h \
		  qnetwork.h \
		  qserversocket.h \
		  qsocket.h \
		  qsocketdevice.h
SOURCES		= qdns.cpp \
		  qftp.cpp \
		  qhostaddress.cpp \
		  qhttp.cpp \
		  qnetwork.cpp \
		  qserversocket.cpp \
		  qsocket.cpp \
		  qsocketdevice.cpp
TARGET		= qnetwork
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.8

unix:SOURCES   += qsocketdevice_unix.cpp
