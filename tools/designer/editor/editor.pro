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
		  arghintwidget.h \
		  cindent.h

SOURCES		= editor.cpp \
		  parenmatcher.cpp  \
		  completion.cpp \
		  viewmanager.cpp \
		  markerwidget.cpp \
		  conf.cpp \
		  browser.cpp \
		  arghintwidget.cpp \
		  cindent.cpp \
		  yyindent.cpp

INTERFACES	= preferences.ui
		
TARGET		= editor
DESTDIR		= ../../../lib
DLLDESTDIR	= ../../../bin
VERSION		= 1.0.0

isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)

INCLUDEPATH	+= $$QT_SOURCE_TREE/src/kernel $$QT_SOURCE_TREE/tools/designer/interfaces
