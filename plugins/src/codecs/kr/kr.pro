GUID 	 = {cf9d81fa-a1bb-4f8d-8670-fa3f0042a7d3}
TEMPLATE = lib
TARGET	 = qkrcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

QTDIR_build:REQUIRES = !bigcodecs

HEADERS		= ../../../../include/qeuckrcodec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeuckrcodec.cpp \
		  ../../../../src/codecs/qfontkrcodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

