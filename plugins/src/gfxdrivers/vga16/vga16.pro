GUID	 = {4cfcfa94-38f8-411b-b8d0-9e0c854d1fc5}
TEMPLATE = lib
TARGET	 = qgfxvga16

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_VGA16
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/qgfxvga16_qws.h
SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvga16_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
