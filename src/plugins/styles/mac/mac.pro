TARGET	 = qmacstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qmacstyle_mac.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qmacstyle_mac.cpp

!contains(styles, windows) {
	HEADERS += ../../../gui/styles/qwindowsstyle.h
	SOURCES += ../../../gui/styles/qwindowsstyle.cpp
}

target.path += $$[QT_INSTALL_PLUGINS]/designer
INSTALLS 	+= target
