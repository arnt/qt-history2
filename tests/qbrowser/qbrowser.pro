TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= helpwindow.h
SOURCES		= helpwindow.cpp \
		  main.cpp
TARGET		= qbrowser
LIBS		= -lqnetwork
INCLUDEPATH	= $(QTDIR)/extensions/network/src
