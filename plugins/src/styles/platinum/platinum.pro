TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qplatinumstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qplatinumstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qplatinumstyle
DESTDIR		= ../../../styles

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/styles
INSTALLS += target
