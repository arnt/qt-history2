TEMPLATE	= lib
CONFIG+= qt warn_off release plugin

HEADERS		= ../../../../include/qgfxtransformed_qws.h 

SOURCES		= main.cpp \
		  ../../../../src/kernel/qgfxtransformed_qws.cpp 

unix:OBJECTS_DIR	= .obj

TARGET		= qgfxtransformed
DESTDIR		= ../../../gfxdrivers
DEFINES		-= QT_NO_QWS_TRANSFORMED

target.path=$$plugins.path/gfxdrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/gfxdrivers
INSTALLS += target
