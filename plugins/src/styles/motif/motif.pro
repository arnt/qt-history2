TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_MOTIF

HEADERS		= ../../../../src/styles/qmotifstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qmotifstyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
