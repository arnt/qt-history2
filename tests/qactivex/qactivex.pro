TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qactivex.h qcomobject.h
SOURCES		= qactivex.cpp qcomobject.cpp plugin.cpp
FORMS		= qactivexselect.ui

INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
DESTDIR		= ../../plugins/designer
DLLDESTDIR	= $(QTDIR)/bin
TARGET		= qactivex
