TEMPLATE	= app
CONFIG		= qt warn_on release
LIBS		= -lqnetwork
INCLUDEPATH	= $(QTDIR)/extensions/network/src
HEADERS	= qws.h qwscommand.h qwsproperty.h qws_gui.h
SOURCES	= qws.cpp qwsproperty.cpp qws_gui.cpp qws_linuxfb.cpp main.cpp
TARGET		= qws
TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
