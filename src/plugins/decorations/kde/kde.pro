TARGET	 = qdecorationkde
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationkde_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationkde_qws.cpp

target.path += $$plugins.path/decorations
INSTALLS += target
