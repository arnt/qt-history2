TEMPLATE	= lib
CONFIG		+= qt warn_on
HEADERS		= qaxwidget.h qcomobject.h ../shared/types.h
SOURCES		= qaxwidget.cpp qcomobject.cpp ../shared/types.cpp
FORMS		= qactivexselect.ui

DESTDIR		= $(QTDIR)/lib
DLLDESTDIR	= $(QTDIR)/bin
TARGET		= qaxwidget

shared {
    CONFIG	+= plugin
    SOURCES	+= plugin.cpp
    DLLDESTDIR	+= $(QTDIR)/plugins/designer
    INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
}

win32-borland:INCLUDEPATH += $(BCB)/include/Atl
