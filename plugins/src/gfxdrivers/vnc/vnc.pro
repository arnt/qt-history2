# Project ID used by some IDEs
GUID 	 = {c26f7e31-df75-4246-851f-61b1149a7842}
TEMPLATE = lib
TARGET	 = qgfxvnc

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_VNC
unix:OBJECTS_DIR	= .obj

HEADERS		= ../../../../include/qgfxvnc_qws.h
SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvnc_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
