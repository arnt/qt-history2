TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_WINDOWS

HEADERS		= ../../qwindowsstyle.h

SOURCES		= main.cpp \
		  ../../qwindowsstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qwindowsstyle
DESTDIR		= $(QTDIR)/plugins
