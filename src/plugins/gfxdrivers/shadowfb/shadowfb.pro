TARGET	 = qgfxshadowfb
include(../../qpluginbase.pri)

DESTDIR		= $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_SHADOWFB

HEADERS		= ../../../../include/Qt/qgfxshadowfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxshadowfb_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
