TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxmach64_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxmach64_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxmach64
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_MACH64

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
