TEMPLATE = lib
TARGET	 = qgfxmach64

CONFIG  += qt warn_off plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_MACH64
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxmach64_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxmach64_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
