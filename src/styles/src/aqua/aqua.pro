TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin
win32:DEFINES	+= QT_DLL QT_PLUGIN_STYLE_AQUA QT_PLUGIN_STYLE_WINDOWS

HEADERS		= ../../qwindowsstyle.h \
		  ../../qaquastyle.h

SOURCES		= main.cpp \
		  ../../qwindowsstyle.cpp \
		  ../../qaquastyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qaquastyle
DESTDIR		= ../../../../plugins/styles

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
