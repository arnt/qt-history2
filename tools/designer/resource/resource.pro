TEMPLATE	= lib
CONFIG		+= qt warn_on release
SOURCES		= qwidgetfactory.cpp \
		  ../shared/widgetdatabase.cpp \
		  ../shared/domtool.cpp \
		  ../integration/kdevelop/kdewidgets.cpp \
HEADERS		= qwidgetfactory.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h
TARGET		= qresource
INCLUDEPATH	= ../shared ../util ../../../src/3rdparty/zlib/
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
