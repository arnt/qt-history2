TARGET	 = qwindowsxpstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qwindowsxpstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qwindowsxpstyle.cpp

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
