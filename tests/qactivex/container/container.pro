TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qactivex.h qcomobject.h ../shared/types.h
SOURCES		= qactivex.cpp qcomobject.cpp plugin.cpp
FORMS		= qactivexselect.ui

INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
DESTDIR		= $(QTDIR)/lib
DLLDESTDIR	= $(QTDIR)/bin $(QTDIR)/plugins/designer
TARGET		= qactivex

win32-borland:INCLUDEPATH += $(BCB)/include/Atl
