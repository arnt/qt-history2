TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qactivex.h qcomobject.h
SOURCES		= qactivex.cpp qcomobject.cpp plugin.cpp
FORMS		= qactivexselect.ui

INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
DESTDIR		= $(QTDIR)/lib
DLLDESTDIR	= $(QTDIR)/bin $(QTDIR)/plugins/designer
TARGET		= qactivex
