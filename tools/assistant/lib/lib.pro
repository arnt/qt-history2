TEMPLATE        = lib
TARGET		= qassistantclient
VERSION		= 1.0
CONFIG		+= qt warn_on release 
SOURCES		= qassistantclient.cpp
HEADERS         += $$QT_SOURCE_TREE/include/qassistantclient.h

CONFIG += staticlib
CONFIG	-= dll

DEFINES		+= QT_INTERNAL_NETWORK
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../lib

unix {
	target.path=$$libs.path

	INSTALLS        += target
}
