TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxmatrox_qws.h \
		  ../../../../include/qgfxmatroxdefs_qws.h

SOURCES		= main.cpp \
		  ../../../../src/kernel/qgfxmatrox_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxmatrox
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_MATROX

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
