TEMPLATE	= lib
CONFIG		= qt warn_on debug
win32:CONFIG    += dll
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_COMPACT QT_PLUGIN_STYLE_WINDOWS

HEADERS		= ../../qwindowsstyle.h \
		  ../../qcompactstyle.h

SOURCES		= main.cpp \
		  ../../qwindowsstyle.cpp \
		  ../../qcompactstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qcompactstyle
DESTDIR		= $(QTDIR)/plugins
