TARGET	 = qgfxmach64
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_MACH64

HEADERS		= ../../../../include/Qt/qgfxmach64_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxmach64_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS 	+= target
