TARGET	 = qcompactstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qcompactstyle.h

SOURCES		= main.cpp \
		  ../../../gui/styles/qcompactstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../gui/styles/qwindowsstyle.h
	SOURCES += ../../../gui/styles/qwindowsstyle.cpp
}

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
