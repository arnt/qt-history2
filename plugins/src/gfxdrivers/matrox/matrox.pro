TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxmatrox_qws.h \
		  ../../../../include/qgfxmatroxdefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxmatrox_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxmatrox
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_MATROX

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
