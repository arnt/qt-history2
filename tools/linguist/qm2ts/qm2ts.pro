TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp

!xml:DEFINES 	+= QT_INTERNAL_XML
else:QT += xml
include( ../../../src/qt_professional.pri )

TARGET		= qm2ts
INCLUDEPATH	+= ../shared
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS	+= target
