TEMPLATE        = lib
TARGET		= qassistantclient
VERSION		= 1.0
CONFIG		+= qt warn_on release 
SOURCES		= qassistantclient.cpp
HEADERS         += $$QT_SOURCE_TREE/include/qassistantclient.h

CONFIG += staticlib
CONFIG	-= dll

!network:DEFINES += QT_INTERNAL_NETWORK
else:QCONFIG += network
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../lib

unix {
	target.path=$$libs.path
	QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
	QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
	INSTALLS        += target
}
