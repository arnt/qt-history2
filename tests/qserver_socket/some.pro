TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= some.h \
		  thing.h
SOURCES		= main.cpp \
		  some.cpp \
		  thing.cpp
INTERFACES	= 
TARGET		= some
unix:LIBS       = -lqnetwork
win32:LIBS      = $(QTDIR)\lib\qnetwork.lib
