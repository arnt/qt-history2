TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h
SOURCES		= main.cpp \
		  ../shared/metatranslator.cpp

DEFINES 	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= qm2ts
INCLUDEPATH	= ../shared
DESTDIR		= ../../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS	+= target
