TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvnc_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvnc_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvnc
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VNC

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/gfxdrivers
INSTALLS 	+= target
