TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_CDE QT_PLUGIN_STYLE_MOTIF

HEADERS		= ../../qmotifstyle.h \
		  ../../qcdestyle.h 

SOURCES		= main.cpp \
		  ../../qmotifstyle.cpp \
		  ../../qcdestyle.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qcdestyle
DESTDIR		= $(QTDIR)/plugins

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
