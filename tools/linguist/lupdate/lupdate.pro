TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h \
		  ../shared/proparser.h
SOURCES		= fetchtr.cpp \
		  main.cpp \
		  merge.cpp \
		  numberh.cpp \
		  sametexth.cpp \
		  ../shared/metatranslator.cpp \
		  ../shared/proparser.cpp
INCLUDEPATH	= ../shared
include( ../../../src/qt_professional.pri )
DESTDIR		= ../../../bin
