TEMPLATE = lib
TARGET	 = qgfxmach64

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	+= QT_QWS_MACH64
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxmach64_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxmach64_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
