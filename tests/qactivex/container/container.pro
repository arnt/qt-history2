TEMPLATE	= lib
CONFIG		+= qt warn_on
HEADERS		= qaxbase.h qaxwidget.h qaxobject.h ../shared/types.h
SOURCES		= qaxbase.cpp qaxwidget.cpp qaxobject.cpp ../shared/types.cpp
FORMS		= qactivexselect.ui

DESTDIR		= $(QTDIR)/lib
DLLDESTDIR	= $(QTDIR)/bin
TARGET		= qaxcontainer

shared {
    CONFIG	+= plugin
    SOURCES	+= plugin.cpp
    DLLDESTDIR	+= $(QTDIR)/plugins/designer
    INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces
}

win32-borland:INCLUDEPATH += $(BCB)/include/Atl
