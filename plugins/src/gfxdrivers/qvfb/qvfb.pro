TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvfb_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvfb_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvfb
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VFB

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
