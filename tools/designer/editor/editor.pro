TEMPLATE	= lib
CONFIG		+= qt warn_on release dll
win32:DEFINES   += EDITOR_DLL
HEADERS		= editor.h  \
		  parenmatcher.h  \
		  completion.h \
		  viewmanager.h \
		  markerwidget.h\
		  conf.h \
		  browser.h \
		  arghintwidget.h

SOURCES		= editor.cpp \
		  parenmatcher.cpp  \
		  completion.cpp \
		  viewmanager.cpp \
		  markerwidget.cpp \
		  conf.cpp \
		  browser.cpp \
		  arghintwidget.cpp

INTERFACES	= preferences.ui
		
TARGET		= editor
DESTDIR		= ../../../lib
DLLDESTDIR	= ../../../bin
VERSION		= 1.0.0
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces
