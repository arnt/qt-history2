TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= nntp.h http.h view.h
SOURCES		= main.cpp \
		  nntp.cpp http.cpp view.cpp
TARGET		= networkprotocol
unix:LIBS	= -lqnetwork
win32:LIBS	= $(QTDIR)/lib/qnetwork.lib
