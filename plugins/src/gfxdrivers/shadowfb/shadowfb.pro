GUID 	 = {4332b8fc-d254-49b5-a65a-0ca2e0f2e0db}
TEMPLATE = lib
TARGET	 = qgfxshadowfb

CONFIG  += qt warn_off release plugin
DESTDIR		= ../../../gfxdrivers

DEFINES	-= QT_NO_QWS_SHADOWFB
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/qgfxshadowfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/embedded/qgfxshadowfb_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
