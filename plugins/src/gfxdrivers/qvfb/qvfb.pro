TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvfb_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/kernel/qgfxvfb_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvfb
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VFB

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
