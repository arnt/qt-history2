TARGET	 = qdecorationbeos
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationbeos_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationbeos_qws.cpp

target.path += $$[QT_INSTALL_PLUGINS]/decorations
INSTALLS += target
