TEMPLATE = lib
TARGET	 = qdecorationhydro

CONFIG  += qt warn_off release plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/decorations

HEADERS		= ../../../gui/embedded/qdecorationhydro_qws.h
SOURCES		= main.cpp \
		  ../../../gui/embedded/qdecorationhydro_qws.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/decorations
INSTALLS += target
