TARGET	 = qplastiquestyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qplastiquestyle.h \
		  ../../../gui/styles/qwindowsstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qplastiquestyle.cpp \
		   ../../../gui/styles/qwindowsstyle.cpp

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
