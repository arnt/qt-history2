TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxshadowfb_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxshadowfb_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxshadowfb
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_SHADOWFB

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
