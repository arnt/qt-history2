GUID 	 = {cd6a5416-76b3-484e-96d3-d2e91d2c8e1e}
TEMPLATE = lib
TARGET	 = qsgistyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qsgistyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qsgistyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../../include/qmotifstyle.h
	SOURCES += ../../../../src/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
