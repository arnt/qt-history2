TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h \
		  ../shared/proparser.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp \
		  ../shared/proparser.cpp
INCLUDEPATH	= ../shared
include( ../../../src/qt_professional.pri )
DESTDIR		= ../../../bin
