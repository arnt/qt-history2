TEMPLATE	= lib
CONFIG		= qt warn_on release plugin
HEADERS	= rc2ui.h
SOURCES	= main.cpp rc2ui.cpp
DESTDIR		= $(QTDIR)/plugins
TARGET		= rcplugin
INCLUDEPATH     += ../../interfaces
