# Project ID used by some IDEs
GUID 	 = {1f137ed1-bc44-44e6-b130-fd8d8d336dff}
TEMPLATE = lib
TARGET	 = qjpcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

QTDIR_build:REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/qeucjpcodec.h \
		  ../../../../include/qjiscodec.h \
		  ../../../../include/qsjiscodec.h \
		  ../../../../include/qjpunicode.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeucjpcodec.cpp \
		  ../../../../src/codecs/qjiscodec.cpp \
		  ../../../../src/codecs/qsjiscodec.cpp \
		  ../../../../src/codecs/qjpunicode.cpp \
		  ../../../../src/codecs/qfontjpcodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

