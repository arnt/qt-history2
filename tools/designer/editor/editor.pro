TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:DEFINES   += EDITOR_DLL
HEADERS		= editor.h  \
		  parenmatcher.h  \
		  completion.h \
		  viewmanager.h \
		  markerwidget.h \
		  preferencesimpl.h

SOURCES		= editor.cpp \
		  parenmatcher.cpp  \
		  completion.cpp \
		  viewmanager.cpp \
		  markerwidget.cpp \
		  preferencesimpl.cpp

INTERFACES	= preferences.ui
		
TARGET		= editor
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces
