TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qmotifstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qmotifstyle
DESTDIR		= ../../../styles


target.path += $$plugins.path/styles
INSTALLS += target
