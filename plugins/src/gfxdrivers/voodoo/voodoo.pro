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

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
