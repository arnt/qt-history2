TEMPLATE	= lib
CONFIG		= qt warn_on release
win32:CONFIG	+= dll

HEADERS	= previewstack.h \
		  styledbutton.h \
		  ../../../qtable/qtable.h \
		  ../../../../include/qdefaultinterface.h

SOURCES	= init.cpp \
		  previewstack.cpp \
		  styledbutton.cpp \
		  ../../../qtable/qtable.cpp		  

TARGET		= myplugin
DESTDIR		= $(QTDIR)/plugins
INCLUDEPATH	+= $(QTDIR)/include