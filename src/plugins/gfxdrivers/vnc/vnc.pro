TARGET	 = qgfxvnc
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VNC

HEADERS		= ../../../../include/Qt/qgfxvnc_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvnc_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
