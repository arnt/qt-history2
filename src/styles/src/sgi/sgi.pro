TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_SGI QT_PLUGIN_STYLE_MOTIF

HEADERS		= ../../qmotifstyle.h \
		  ../../qsgistyle.h 

SOURCES		= main.cpp \
		  ../../qmotifstyle.cpp \
		  ../../qsgistyle.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qsgistyle
DESTDIR		= $(QTDIR)/plugins
