GUID 		= {403c89b7-cc9b-478e-96ce-06db15afe6ce}
TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp

!xml:DEFINES 	+= QT_INTERNAL_XML
else:QCONFIG += xml
include( ../../../src/qt_professional.pri )

TARGET		= qm2ts
INCLUDEPATH	+= ../shared
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS	+= target
