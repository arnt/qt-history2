TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= test
unix:LIBS		+= -lqui -L$(QTDIR)/lib
win32:LIBS	+= $(QTDIR)/lib/qui.lib

