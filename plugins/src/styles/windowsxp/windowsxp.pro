GUID 	 = {ad5fcaa1-5602-4160-bf1f-0abab44cc72d}
TEMPLATE = lib
TARGET	 = qwindowsxpstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qwindowsxpstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
