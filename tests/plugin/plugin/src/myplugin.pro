TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= dll

HEADERS	= previewstack.h \
		  styledbutton.h \
		  ../../qdefaultinterface.h

SOURCES	= init.cpp \
		  previewstack.cpp \
		  styledbutton.cpp \

TARGET		= myplugin
DESTDIR		= $(QTDIR)/plugins
INCLUDEPATH	+= $(QTDIR)/include
