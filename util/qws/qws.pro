TEMPLATE	= app
CONFIG		= qt warn_on release
LIBS		= -lqnetwork
INCLUDEPATH	= $(QTDIR)/extensions/network/src
HEADERS	= qws.h qwscommand.h qwsproperty.h
SOURCES	= qws.cpp qwscommand.cpp qwsproperty.cpp
TARGET		= qws
