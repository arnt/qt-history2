TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= test
unix:LIBS		+= -lqresource -L$(QTDIR)/lib
win32:LIBS	+= $(QTDIR)/lib/qresource.lib

