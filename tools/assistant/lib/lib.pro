TEMPLATE        = lib
TARGET		= qassistantclient
VERSION		= 1.0
CONFIG		+= qt warn_on release 
SOURCES		= qassistantclient.cpp

win32:CONFIG	+= static
win32:CONFIG	-= dll

include( ../../../src/qt_professional.pri )
DESTDIR		= ../../../lib

unix {
	target.path=$$libs.path

	INSTALLS        += target
}
