TEMPLATE = lib
TARGET	 = qgfxshadowfb

CONFIG  += qt warn_off plugin
DESTDIR		= $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_SHADOWFB
unix:OBJECTS_DIR = .obj

HEADERS		= ../../../../include/Qt/qgfxshadowfb_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxshadowfb_qws.cpp


target.path += $$plugins.path/gfxdrivers
INSTALLS += target
