TARGET	 = qgfxvfb
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VFB

HEADERS		= ../../../../include/Qt/qgfxvfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvfb_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
