TEMPLATE = lib
TARGET	 = qgfxtransformed

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_TRANSFORMED
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxtransformed_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxtransformed_qws.cpp


target.path=$$plugins.path/gfxdrivers
INSTALLS += target
