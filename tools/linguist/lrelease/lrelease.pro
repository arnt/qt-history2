# Project ID used by some IDEs
GUID 		= {7f6e0508-2a63-4c26-b07a-62f683f46570}
TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h \
		  ../shared/proparser.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp \
		  ../shared/proparser.cpp

!xml:DEFINES 	+= QT_INTERNAL_XML
else:QCONFIG += xml
include( ../../../src/qt_professional.pri )

TARGET		= lrelease
INCLUDEPATH	+= ../shared
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS	+= target
