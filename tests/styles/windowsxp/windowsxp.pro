TEMPLATE	= lib
CONFIG		+= qt warn_on debug plugin
DEFINES		+= QT_DLL QT_PLUGIN_STYLE_WINDOWSXP

HEADERS		= windowsxpstyle.h

SOURCES		= main.cpp \
		  windowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= windowsxpstyle
DESTDIR		= ../../../plugins/styles
LIBS		+= uxtheme.lib delayimp.lib
QMAKE_LFLAGS    += /DELAYLOAD:uxtheme.dll

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
