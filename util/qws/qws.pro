TEMPLATE	= app
CONFIG		= qt warn_on release
LIBS		= -lqnetwork
TMAKE_INCDIR	+= $(QTDIR)/extensions/network/src
HEADERS		= qtfb.h
SOURCES		= qtfb.cpp
TARGET		= qtfb
