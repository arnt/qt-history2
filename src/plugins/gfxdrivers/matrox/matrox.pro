TEMPLATE = lib
TARGET	 = qgfxmatrox

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_MATROX
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxmatrox_qws.h \
		  ../../../../include/Qt/qgfxmatroxdefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxmatrox_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
