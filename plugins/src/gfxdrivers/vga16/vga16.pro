TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvga16_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvga16_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvga16
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VGA16

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
