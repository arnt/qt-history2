TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxvga16_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxvga16_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxvga16
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_VGA16

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
