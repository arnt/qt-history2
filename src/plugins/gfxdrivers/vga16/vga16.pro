TEMPLATE = lib
TARGET	 = qgfxvga16

CONFIG  += qt warn_off plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	+= QT_QWS_VGA16
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxvga16_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvga16_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
