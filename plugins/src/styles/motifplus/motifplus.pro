TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_MOTIFPLUS QT_PLUGIN_STYLE_MOTIF

HEADERS		= ../../../../src/styles/qmotifstyle.h \
		  ../../../../src/styles/qmotifplusstyle.h 

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
