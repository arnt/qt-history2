TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_PLATINUM QT_PLUGIN_STYLE_WINDOWS

HEADERS		= ../../qwindowsstyle.h \
		  ../../qplatinumstyle.h

SOURCES		= main.cpp \
		  ../../qwindowsstyle.cpp \
		  ../../qplatinumstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qplatinumstyle
DESTDIR		= $(QTDIR)/plugins/styles

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
