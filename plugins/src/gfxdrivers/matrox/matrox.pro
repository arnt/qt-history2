# Project ID used by some IDEs
GUID 	 = {9b9dec2e-1118-4666-979e-3acf319893d8}
TEMPLATE = lib
TARGET	 = qgfxmatrox

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_MATROX
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/qgfxmatrox_qws.h \
		  ../../../../include/qgfxmatroxdefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxmatrox_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
