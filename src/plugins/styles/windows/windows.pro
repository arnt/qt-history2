TARGET	 = qwindowsstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qwindowsstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qwindowsstyle.cpp

target.path += $$plugins.path/styles
INSTALLS += target
