TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp
INCLUDEPATH	= ../shared
include( ../../../src/qt_professional.pri )
DESTDIR	= ../../../bin
