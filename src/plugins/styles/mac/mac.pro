TEMPLATE = lib
TARGET	 = qmacstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qmacstyle_mac.h \
		  ../../../gui/styles/qmacstyle_mac.h \
		  ../../../gui/styles/qmacstyleqd_mac.h \
		  ../../../gui/styles/qmacstylecg_mac.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qmacstyle_mac.cpp \
		  ../../../gui/styles/qmacstyleqd_mac.cpp \
		  ../../../gui/styles/qmacstylecg_mac.cpp

!contains(styles, windows) {
	HEADERS += ../../../gui/styles/qwindowsstyle.h
	SOURCES += ../../../gui/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/designer
INSTALLS 	+= target
