TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:DEFINES   += CPP_DLL
HEADERS		= editor.h  \
		  parenmatcher.h  \
		  completion.h \
		  viewmanager.h \
		  markerwidget.h

SOURCES		= editor.cpp \
		  parenmatcher.cpp  \
		  completion.cpp \
		  viewmanager.cpp \
		  markerwidget.cpp
		
TARGET		= editor
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces
