TARGET	 = qdecorationdefault
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationdefault_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationdefault_qws.cpp

target.path += $$plugins.path/decorations
INSTALLS += target
