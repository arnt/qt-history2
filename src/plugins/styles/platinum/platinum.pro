TEMPLATE = lib
TARGET	 = qplatinumstyle

CONFIG  += qt warn_off release plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qplatinumstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qplatinumstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../gui/styles/qwindowsstyle.h
	SOURCES += ../../../gui/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
