# Project ID used by some IDEs
GUID 	 = {56972c67-ca0d-496e-a3fe-5e33e4c11227}
TEMPLATE = lib
TARGET	 = qcdestyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qcdestyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qcdestyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../../include/qmotifstyle.h
	SOURCES += ../../../../src/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
