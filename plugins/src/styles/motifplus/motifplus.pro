TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qmotifplusstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifplusstyle.cpp 

!contains(styles, motif) {
	HEADERS += ../../../../include/qmotifstyle.h 
	SOURCES += ../../../../src/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qmotifplusstyle
DESTDIR		= ../../../styles

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/styles
INSTALLS += target
