TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvoodoo_qws.h \
		  ../../../../include/qgfxvoodoodefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvoodoo_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvoodoo
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VOODOO3

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
