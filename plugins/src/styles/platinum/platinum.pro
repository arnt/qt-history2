TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_PLATINUM QT_PLUGIN_STYLE_WINDOWS

HEADERS		= ../../../../src/styles/qwindowsstyle.h \
		  ../../../../src/styles/qplatinumstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsstyle.cpp \
		  ../../../../src/styles/qplatinumstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qplatinumstyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
