TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
win32:CONFIG	-= dll
SOURCES		= qwidgetfactory.cpp \
		  ../shared/domtool.cpp

HEADERS		= qwidgetfactory.h \
		  ../shared/domtool.h

sql:SOURCES += 		  ../designer/database.cpp
sql:HEADERS +=		  ../designer/database2.h

DEFINES += QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )
TARGET		= qui
INCLUDEPATH	+= ../shared ../../../src/3rdparty/zlib/
DESTDIR		= ../../../lib
VERSION		= 1.0.0
DEFINES		+= RESOURCE

unix {
	target.path=$$QT_INSTALL_LIBPATH
	isEmpty(target.path):target.path=$$QT_PREFIX/lib
	INSTALLS        += target
}
