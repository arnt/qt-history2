TARGET	 = qgfxmatrox
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_MATROX

HEADERS		= ../../../../include/Qt/qgfxmatrox_qws.h \
		  ../../../../include/Qt/qgfxmatroxdefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxmatrox_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
