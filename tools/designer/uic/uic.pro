TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= uic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h

SOURCES	= main.cpp uic.cpp form.cpp object.cpp \ 
		   subclassing.cpp embed.cpp\
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp

DEFINES		+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= uic
INCLUDEPATH	+= ../shared ../../../src/3rdparty/zlib/
!zlib:unix:LIBS	+= -lz
DEFINES 	+= UIC
DESTDIR		= ../../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
