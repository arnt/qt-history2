# Project ID used by some IDEs
GUID 	 = {6d643b23-919c-4cfb-ba36-0f82ad0c4114}
TEMPLATE = lib
TARGET	 = qmacstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qmacstyle_mac.h \
		  ../../../../src/styles/qmacstyle_mac.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qmacstyle_mac.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/designer
INSTALLS 	+= target
