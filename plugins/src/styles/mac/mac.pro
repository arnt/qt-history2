TEMPLATE = lib
TARGET	 = qmacstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qmacstyle_mac.h \
		  ../../../../src/styles/qmacstyle_mac.h \
		  ../../../../src/styles/qmacstyleqd_mac.h \
		  ../../../../src/styles/qmacstylecg_mac.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qmacstyle_mac.cpp \
		  ../../../../src/styles/qmacstyleqd_mac.cpp \
		  ../../../../src/styles/qmacstylecg_mac.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/designer
INSTALLS 	+= target
