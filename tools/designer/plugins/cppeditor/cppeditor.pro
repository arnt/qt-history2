TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= dll
win32:DEFINES   += CPP_DLL
HEADERS		= editor.h  \
		  parenmatcher.h  \
		  syntaxhighliter_cpp.h \
		  indent_cpp.h \
		  completion.h \
		  iconloader.h \
		  viewmanager.h \
		  editorinterfaceimpl.h \
		  markerwidget.h

SOURCES		= editor.cpp \
		  parenmatcher.cpp  \
		  syntaxhighliter_cpp.cpp \
		  indent_cpp.cpp \
		  completion.cpp \
		  iconloader.cpp \
		  viewmanager.cpp \
		  editorinterfaceimpl.cpp \
		  markerwidget.cpp
		
TARGET		= cppeditor
DESTDIR		= $(QTDIR)/plugins
VERSION		= 1.0.0
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces
