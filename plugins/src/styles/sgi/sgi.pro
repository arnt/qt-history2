TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qsgistyle.h 

SOURCES		= main.cpp \
		  ../../../../src/styles/qsgistyle.cpp 

!contains(styles, motif) {
	HEADERS += ../../../../include/qmotifstyle.h
	SOURCES += ../../../../src/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qsgistyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
