# Project ID used by some IDEs
GUID 	 = {b0af524f-b4fc-40c2-bdd5-0652a30fdc31}
TEMPLATE = lib
TARGET	 = qgfxvfb

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_VFB
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/qgfxvfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvfb_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
