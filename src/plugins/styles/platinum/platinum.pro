TARGET	 = qplatinumstyle
include(../../qpluginbase.pri)

CONFIG  += qt warn_off plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qplatinumstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qplatinumstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../gui/styles/qwindowsstyle.h
	SOURCES += ../../../gui/styles/qwindowsstyle.cpp
}

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
