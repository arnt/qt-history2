TEMPLATE	= lib
CONFIG		= qt warn_on release thread
WIN32:CONFIG   += dll
HEADERS		=
SOURCES		= main.cpp
DESTDIR		= $(QTDIR)/plugins
TARGET		= threadplugin
