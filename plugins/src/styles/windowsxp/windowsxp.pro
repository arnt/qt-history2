TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qwindowsxpstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qwindowsxpstyle
DESTDIR		= ../../../styles


target.path += $$plugins.path/styles
INSTALLS += target
