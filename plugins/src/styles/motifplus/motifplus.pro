TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../include/qmotifstyle.h \
		  ../../../../include/qmotifplusstyle.h 

SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifstyle.cpp \
		  ../../../../src/styles/qmotifplusstyle.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qmotifplusstyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
