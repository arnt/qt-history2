TARGET	 = qgfxvfb
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_QVFB

HEADERS		= ../../../../include/Qt/qgfxvfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvfb_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
