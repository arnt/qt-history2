TEMPLATE = lib
TARGET	 = qgfxvnc

CONFIG  += qt warn_off plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_VNC
unix:OBJECTS_DIR	= .obj

HEADERS		= ../../../../include/Qt/qgfxvnc_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxvnc_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
