TEMPLATE	= lib
CONFIG		= qt warn_on release
WIN32:CONFIG   += dll
HEADERS		= glwidget.h
SOURCES		= main.cpp \
		  glwidget.cpp
INTERFACES	=
DESTDIR		= $(QTDIR)/plugins
