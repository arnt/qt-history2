TEMPLATE	= lib
CONFIG		= qt warn_on release
WIN32:CONFIG   += dll
HEADERS	= rc2ui.h
SOURCES	= main.cpp rc2ui.cpp
DESTDIR		= $(QTDIR)/plugins
TARGET		= rcplugin
