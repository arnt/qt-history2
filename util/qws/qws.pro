TEMPLATE	= app
CONFIG		+= qt warn_on release
DEFINES 	+= QWS
LIBS		= -lqnetwork
INCLUDEPATH	= $$QT_SOURCE_TREE/extensions/network/src \
                  $$QT_SOURCE_TREE/include \
                  $$QT_SOURCE_TREE/../e/src/kernel \
                  $$QT_SOURCE_TREE/../e/include
HEADERS	= qws.h qwscommand.h qwsproperty.h qws_gui.h qwsaccel.h \
	  qwsmach64.h qwsmach64defs.h qws_cursor.h
SOURCES	= qws.cpp qwsproperty.cpp qws_gui.cpp qws_linuxfb.cpp main.cpp \
	  qwsaccel.cpp qwsmach64.cpp qws_linuxmouse.cpp qws_linuxkb.cpp \
	  qws_cursor.cpp
TARGET		= qws
!win32:TMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
