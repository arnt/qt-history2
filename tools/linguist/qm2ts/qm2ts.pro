TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp
INCLUDEPATH	= ../shared
DESTDIR	= $(QTDIR)/bin
