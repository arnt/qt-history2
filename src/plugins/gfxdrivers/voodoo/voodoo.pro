TARGET	 = qgfxvoodoo
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VOODOO

HEADERS		= ../../../../include/Qt/qgfxvoodoo_qws.h \
		  ../../../../include/Qt/qgfxvoodoodefs_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvoodoo_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS 	+= target
