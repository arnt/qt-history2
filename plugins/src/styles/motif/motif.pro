GUID 	 = {4f4defd7-c35b-44e5-a898-d809e26f2645}
TEMPLATE = lib
TARGET	 = qmotifstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qmotifstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
