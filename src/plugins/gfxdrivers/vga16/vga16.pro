TARGET	 = qgfxvga16
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VGA16

HEADERS		= ../../../../include/Qt/qgfxvga16_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvga16_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS 	+= target
