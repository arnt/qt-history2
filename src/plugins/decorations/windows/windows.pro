TARGET	 = qdecorationwindows
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationwindows_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationwindows_qws.cpp

target.path += $$plugins.path/decorations
INSTALLS += target
