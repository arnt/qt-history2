TEMPLATE = lib
TARGET	 = qwindowsxpstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qwindowsxpstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qwindowsxpstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
