TEMPLATE	= app
CONFIG		= qt warn_on release
LIBS		= -lqnetwork
INCLUDEPATH	= $(QTDIR)/extensions/network/src
HEADERS	= qws.h qwscommand.h
SOURCES	= qws.cpp qwscommand.cpp
TARGET		= qws
