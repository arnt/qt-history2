TEMPLATE = lib
TARGET	 = qgfxvfb

CONFIG  += qt warn_off plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_VFB
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxvfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvfb_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
