TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qmacstyle_mac.h \
		  ../../../../src/styles/qmacstyle_mac.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qmacstyle_mac.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qmacstyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
