TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qaxwidget.h qcomobject.h ../shared/types.h
SOURCES		= qaxwidget.cpp qcomobject.cpp plugin.cpp ../shared/types.cpp
FORMS		= qactivexselect.ui

INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
DESTDIR		= $(QTDIR)/lib
DLLDESTDIR	= $(QTDIR)/bin $(QTDIR)/plugins/designer
TARGET		= qaxwidget

win32-borland:INCLUDEPATH += $(BCB)/include/Atl
