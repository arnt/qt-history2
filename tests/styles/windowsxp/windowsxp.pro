TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= windowsxpstyle.h

SOURCES		= main.cpp \
		  windowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qwindowsxpstyle
DESTDIR		= ../../../plugins/styles
LIBS		+= uxtheme.lib
win32-msvc:{
    LIBS	+= delayimp.lib
    QMAKE_LFLAGS += /DELAYLOAD:uxtheme.dll
}

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
