TEMPLATE	= lib
CONFIG		= qt staticlib release
TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
HEADERS		= qdns.h \
		  qftp.h \
		  qhttp.h \
		  qhostaddress.h \
		  qnetwork.h \
		  qserversocket.h \
		  qsocket.h \
		  qsocketdevice.h
SOURCES		= qdns.cpp \
		  qftp.cpp \
		  qhttp.cpp \
		  qhostaddress.cpp \
		  qnetwork.cpp \
		  qserversocket.cpp \
		  qsocket.cpp \
		  qsocketdevice.cpp
TARGET		= qnetwork
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.1
