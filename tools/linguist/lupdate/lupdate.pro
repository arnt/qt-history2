GUID 		= {60f2912a-12a8-463e-96e8-cc04b0b64a7a}
TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h \
		  ../shared/proparser.h
SOURCES		= fetchtr.cpp \
		  main.cpp \
		  merge.cpp \
		  numberh.cpp \
		  sametexth.cpp \
		  ../shared/metatranslator.cpp \
		  ../shared/proparser.cpp

!xml:DEFINES 	+= QT_INTERNAL_XML
else:QCONFIG += xml
include( ../../../src/qt_professional.pri )

TARGET		= lupdate
INCLUDEPATH	+= ../shared
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS	+= target
