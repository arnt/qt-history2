TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp
include( ../../../src/qt_professional.pri )
INCLUDEPATH	= ../shared
DESTDIR	= ../../../bin
