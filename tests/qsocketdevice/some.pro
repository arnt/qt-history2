TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= some.h
SOURCES		= main.cpp \
		  some.cpp
INTERFACES	= 
TARGET		= some
unix:LIBS       = -lqnetwork
win32:LIBS      = $(QTDIR)\lib\qnetwork.lib
