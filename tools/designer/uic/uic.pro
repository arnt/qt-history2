TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= uic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h \
		  ../shared/ui2uib.h \
		  ../shared/uib.h

SOURCES	= main.cpp uic.cpp form.cpp object.cpp \
		   subclassing.cpp embed.cpp\
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp \
		  ../shared/ui2uib.cpp \
		  ../shared/uib.cpp

DEFINES		+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= uic
INCLUDEPATH	+= ../shared ../../../src/3rdparty/zlib/
unix:!zlib:LIBS	+= -lz
mac:!system-zlib:LIBS	+= -lz
DEFINES 	+= UIC
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS        += target
