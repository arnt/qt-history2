TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= qwindowsxpstyle.h

SOURCES		= main.cpp \
		  qwindowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qwindowsxpstyle
DESTDIR		= ../../../styles
LIBS		+= uxtheme.lib
win32-msvc:{
    LIBS	+= delayimp.lib
    QMAKE_LFLAGS += /DELAYLOAD:uxtheme.dll
}
win32-borland:{
    QMAKE_LFLAGS += /duxtheme.dll
}

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
