TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxshadowfb_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxshadowfb_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxshadowfb
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_SHADOWFB

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
