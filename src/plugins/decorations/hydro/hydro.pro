TARGET	 = qdecorationhydro
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationhydro_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationhydro_qws.cpp

target.path += $$[QT_INSTALL_PLUGINS]/decorations
INSTALLS += target
