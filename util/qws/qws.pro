TEMPLATE	= app
CONFIG		+= qt warn_on release
DEFINES += QWS
LIBS		= -lqnetwork
INCLUDEPATH	= $(QTDIR)/extensions/network/src $(QTDIR)/include $(QTDIR)/../e/src/kernel $(QTDIR)/../e/include
HEADERS	= qws.h qwscommand.h qwsproperty.h qws_gui.h qwsaccel.h \
	  qwsmach64.h qwsmach64defs.h qws_cursor.h
SOURCES	= qws.cpp qwsproperty.cpp qws_gui.cpp qws_linuxfb.cpp main.cpp \
	  qwsaccel.cpp qwsmach64.cpp qws_linuxmouse.cpp qws_linuxkb.cpp \
	  qws_cursor.cpp
TARGET		= qws
TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
