TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
TARGET		= networkprotocol
unix:LIBS	= -lqnetwork
win32:LIBS	= $(QTDIR)/qnetwork.lib
