TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../src/styles/qmotifstyle.h \
		  ../../../../src/styles/qcdestyle.h 

SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifstyle.cpp \
		  ../../../../src/styles/qcdestyle.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qcdestyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
