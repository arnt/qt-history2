TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qaquastyle.h \
		  ../../../../src/styles/qaquastyle_p.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qaquastyle.cpp \
		  ../../../../src/styles/qaquastyle_p.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qaquastyle
DESTDIR		= ../../../styles


target.path += $$plugins.path/styles
INSTALLS += target
